// Copyright (C) 2023 Yixue Wang <wangyixue@deepin.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "winputmethodhelper.h"
#include "wtextinputv3_p.h"
#include "wtextinputv1_p.h"
#include "wtextinput_p.h"
#include "winputmethodv2_p.h"
#include "wvirtualkeyboardv1_p.h"
#include "winputpopupsurface.h"
#include "wseat.h"
#include "wsurface.h"
#include "private/wglobal_p.h"

#include <qwcompositor.h>
#include <qwinputmethodv2.h>
#include <qwtextinputv3.h>
#include <qwvirtualkeyboardv1.h>
#include <qwseat.h>

#include <QLoggingCategory>
#include <QQmlInfo>

extern "C" {
#define static
#include <wlr/types/wlr_compositor.h>
#undef static
#include <wlr/types/wlr_text_input_v3.h>
#define delete delete_c
#include <wlr/types/wlr_input_method_v2.h>
#undef delete
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/types/wlr_seat.h>
}

QW_USE_NAMESPACE
WAYLIB_SERVER_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(qLcInputMethod, "waylib.server.im", QtInfoMsg)

struct GrabHandlerArg {
    const WInputMethodHelper *const helper;
    QWInputMethodKeyboardGrabV2 *grab;
};

void handleKey(struct wlr_seat_keyboard_grab *grab, uint32_t time_msec, uint32_t key, uint32_t state)
{
    auto arg = reinterpret_cast<GrabHandlerArg*>(grab->data);
    for (auto vk: arg->helper->virtualKeyboards()) {
        if (wlr_keyboard_from_input_device(vk->handle()->handle()) == grab->seat->keyboard_state.keyboard) {
            grab->seat->keyboard_state.default_grab->interface->key(grab, time_msec, key, state);
            return;
        }
    }
    arg->grab->sendKey(time_msec, Qt::Key(key), state);
}

void handleModifiers(struct wlr_seat_keyboard_grab *grab, const struct wlr_keyboard_modifiers *modifiers)
{
    auto arg = reinterpret_cast<GrabHandlerArg*>(grab->data);
    for (auto vk: arg->helper->virtualKeyboards()) {
        if (wlr_keyboard_from_input_device(vk->handle()->handle()) == grab->seat->keyboard_state.keyboard) {
            grab->seat->keyboard_state.default_grab->interface->modifiers(grab, modifiers);
            return;
        }
    }
    arg->grab->sendModifiers(const_cast<struct wlr_keyboard_modifiers *>(modifiers));
}

class WInputMethodHelperPrivate : public WObjectPrivate
{
    W_DECLARE_PUBLIC(WInputMethodHelper)
public:
    explicit WInputMethodHelperPrivate(WServer *s, WSeat *st, WInputMethodHelper *qq)
        : WObjectPrivate(qq)
        , server(s)
        , seat(st)
        , inputMethodManagerV2(server->attach<WInputMethodManagerV2>())
        , textInputManagerV1(server->attach<WTextInputManagerV1>())
        , textInputManagerV3(server->attach<WTextInputManagerV3>())
        , virtualKeyboardManagerV1(server->attach<WVirtualKeyboardManagerV1>())
        , activeTextInput(nullptr)
        , activeInputMethod(nullptr)
        , activeKeyboardGrab(nullptr)
        , keyboardGrab()
        , grabInterface()
        , handlerArg({.helper = qq})
    {
        Q_ASSERT(server);
        Q_ASSERT(seat);
        Q_ASSERT(inputMethodManagerV2);
        Q_ASSERT(textInputManagerV3);
        Q_ASSERT(textInputManagerV1);

    }

    const QPointer<WServer> server;
    const QPointer<WSeat> seat;
    const QPointer<WInputMethodManagerV2> inputMethodManagerV2;
    const QPointer<WTextInputManagerV1> textInputManagerV1;
    const QPointer<WTextInputManagerV3> textInputManagerV3;
    const QPointer<WVirtualKeyboardManagerV1> virtualKeyboardManagerV1;
    WTextInput *activeTextInput { nullptr };
    WInputMethodV2 *activeInputMethod { nullptr };
    QWInputMethodKeyboardGrabV2 *activeKeyboardGrab {nullptr};

    wlr_seat_keyboard_grab keyboardGrab;
    wlr_keyboard_grab_interface grabInterface;
    GrabHandlerArg handlerArg;

    QList<WTextInput *> textInputs;
    QList<WInputDevice *> virtualKeyboards;
    QList<WInputPopupSurface *> popupSurfaces;
};

WInputMethodHelper::WInputMethodHelper(WServer *server, WSeat *seat)
    : QObject(server)
    , WObject(*new WInputMethodHelperPrivate(server, seat, this))
{
    W_D(WInputMethodHelper);
    d->seat->safeConnect(&WSeat::keyboardFocusSurfaceChanged, this, &WInputMethodHelper::resendKeyboardFocus);
    connect(d->inputMethodManagerV2, &WInputMethodManagerV2::newInputMethod, this, &WInputMethodHelper::handleNewIMV2);
    connect(d->textInputManagerV3, &WTextInputManagerV3::newTextInput, this, &WInputMethodHelper::handleNewTI);
    connect(d->virtualKeyboardManagerV1, &WVirtualKeyboardManagerV1::newVirtualKeyboard, this, &WInputMethodHelper::handleNewVKV1);
    connect(d->textInputManagerV1, &WTextInputManagerV1::newTextInput, this, &WInputMethodHelper::handleNewTI);
}

WInputMethodHelper::~WInputMethodHelper()
{
    W_D(WInputMethodHelper);
    if (d->seat) d->seat->safeDisconnect(this);
    if (d->inputMethodManagerV2) d->inputMethodManagerV2->disconnect(this);
    if (d->textInputManagerV1) d->textInputManagerV1->disconnect(this);
    if (d->textInputManagerV3) d->textInputManagerV3->disconnect(this);
    if (d->virtualKeyboardManagerV1) d->virtualKeyboardManagerV1->disconnect(this);
}


WTextInput *WInputMethodHelper::focusedTextInput() const
{
    W_DC(WInputMethodHelper);
    return d->activeTextInput;
}

void WInputMethodHelper::setFocusedTextInput(WTextInput *ti)
{
    W_D(WInputMethodHelper);
    if (d->activeTextInput == ti)
        return;
    if (d->activeTextInput) {
        disconnect(d->activeTextInput, &WTextInput::committed, this, &WInputMethodHelper::handleFocusedTICommitted);
    }
    d->activeTextInput = ti;
    if (ti) {
        updateAllPopupSurfaces(ti->cursorRect()); // Note: if this is necessary
        connect(ti, &WTextInput::committed, this, &WInputMethodHelper::handleFocusedTICommitted, Qt::UniqueConnection);
    }
}

WInputMethodV2 *WInputMethodHelper::inputMethod() const
{
    W_DC(WInputMethodHelper);
    return d->activeInputMethod;
}

void WInputMethodHelper::setInputMethod(WInputMethodV2 *im)
{
    W_D(WInputMethodHelper);
    if (d->activeInputMethod == im)
        return;
    if (d->activeInputMethod)
        d->activeInputMethod->safeDisconnect(this);
    d->activeInputMethod = im;
}

QWInputMethodKeyboardGrabV2 *WInputMethodHelper::activeKeyboardGrab() const
{
    W_DC(WInputMethodHelper);
    return d->activeKeyboardGrab;
}

QList<WInputDevice *> WInputMethodHelper::virtualKeyboards() const
{
    W_DC(WInputMethodHelper);
    return d->virtualKeyboards;
}

void WInputMethodHelper::handleNewIMV2(QWInputMethodV2 *imv2)
{
    W_D(WInputMethodHelper);
    auto wimv2 = new WInputMethodV2(imv2, this);
    if (d->seat->name() != wimv2->seat()->name())
        return;
    if (inputMethod()) {
        qCWarning(qLcInputMethod) << "Ignore second creation of input on the same seat.";
        wimv2->sendUnavailable();
        return;
    }
    setInputMethod(wimv2);
    connect(wimv2, &WInputMethodV2::committed, this, &WInputMethodHelper::handleIMCommitted);
    connect(wimv2, &WInputMethodV2::newKeyboardGrab, this, &WInputMethodHelper::handleNewKGV2);
    connect(wimv2, &WInputMethodV2::newPopupSurface, this, &WInputMethodHelper::handleNewIPSV2);
    // Once input method is online, try to resend enter to textInput
    resendKeyboardFocus();
    // For text input v1, when after sendEnter, enabled signal will be emitted
    wimv2->safeConnect(&QWInputMethodV2::beforeDestroy, this, [this, wimv2]{
        if (inputMethod() == wimv2) {
            setInputMethod(nullptr);
        }
        wimv2->safeDeleteLater();
        notifyLeave();
    });

}

void WInputMethodHelper::handleNewKGV2(QWInputMethodKeyboardGrabV2 *kgv2)
{
    W_D(WInputMethodHelper);
    Q_ASSERT(d->seat);
    auto endGrab = [this, d](QWInputMethodKeyboardGrabV2 *kgv2) {
        if (!d->seat)
            return;
        if (kgv2->handle()->keyboard) {
            d->seat->handle()->keyboardSendModifiers(&kgv2->handle()->keyboard->modifiers);
        }
        d->seat->handle()->keyboardEndGrab();
    };
    auto setKeyboard = [this, d](QWInputMethodKeyboardGrabV2 *kgv2, WInputDevice *keyboard) {
        if (keyboard) {
            auto qwKeyboard = qobject_cast<QWKeyboard *>(keyboard->handle());
            auto *virtualKeyboard = qobject_cast<QWVirtualKeyboardV1 *>(qwKeyboard);
            // refer to:
            // https://github.com/swaywm/sway/blob/master/sway/input/keyboard.c#L391
            if (virtualKeyboard
                && wl_resource_get_client(virtualKeyboard->handle()->resource)
                    == wl_resource_get_client(kgv2->handle()->resource)) {
                return;
            }
            kgv2->setKeyboard(qwKeyboard);
        } else {
            kgv2->setKeyboard(nullptr);
        }
    };
    if (auto activeKG = activeKeyboardGrab()) {
        endGrab(activeKG);
    }
    d->activeKeyboardGrab = kgv2;
    setKeyboard(kgv2, d->seat->keyboard());
    connect(d->seat, &WSeat::keyboardChanged, kgv2, [setKeyboard, d, kgv2](){
        setKeyboard(kgv2, d->seat->keyboard());
    });
    d->grabInterface = *d->seat->nativeHandle()->keyboard_state.grab->interface;
    d->grabInterface.key = handleKey;
    d->grabInterface.modifiers = handleModifiers;
    d->keyboardGrab.seat = d->seat->nativeHandle();
    d->handlerArg.grab = kgv2;
    d->keyboardGrab.data = &d->handlerArg;
    d->keyboardGrab.interface = &d->grabInterface;
    d->seat->handle()->keyboardStartGrab(&d->keyboardGrab);
    connect(kgv2, &QWInputMethodKeyboardGrabV2::beforeDestroy, this, [this, d, endGrab, kgv2] {
        if (activeKeyboardGrab() == kgv2) {
            endGrab(kgv2);
            d->activeKeyboardGrab = nullptr;
        }
    });
}

void WInputMethodHelper::handleNewIPSV2(QWInputPopupSurfaceV2 *ipsv2)
{
    W_D(WInputMethodHelper);

    auto createPopupSurface = [this, d] (WSurface *focus, QRect cursorRect, QWInputPopupSurfaceV2 *popupSurface){
        auto surface = new WInputPopupSurface(popupSurface, focus, this);
        d->popupSurfaces.append(surface);
        Q_EMIT inputPopupSurfaceV2Added(surface);
        updatePopupSurface(surface, cursorRect);
        surface->safeConnect(&QWInputPopupSurfaceV2::beforeDestroy, this, [this, d, surface](){
            d->popupSurfaces.removeAll(surface);
            Q_EMIT inputPopupSurfaceV2Removed(surface);
            surface->safeDeleteLater();
        });
    };

    if (auto ti = focusedTextInput()) {
        createPopupSurface(ti->focusedSurface(), ti->cursorRect(), ipsv2);
    }
}

void WInputMethodHelper::handleNewVKV1(QWVirtualKeyboardV1 *vkv1)
{
    W_D(WInputMethodHelper);
    WInputDevice *keyboard = new WInputDevice(vkv1);
    d->virtualKeyboards.append(keyboard);
    d->seat->attachInputDevice(keyboard);
    connect(vkv1, &QWVirtualKeyboardV1::beforeDestroy, this, [d, this, keyboard] () {
        if (d->seat) d->seat->detachInputDevice(keyboard);
        d->virtualKeyboards.removeOne(keyboard);
        keyboard->safeDeleteLater();
    });
}

void WInputMethodHelper::resendKeyboardFocus()
{
    W_D(WInputMethodHelper);
    notifyLeave();
    auto focus = d->seat->keyboardFocusSurface();
    if (!focus)
        return;
    for (auto textInput : d->textInputs) {
        if (focus->waylandClient() == textInput->waylandClient()) {
            textInput->sendEnter(focus);
        }
    }
}


void WInputMethodHelper::connectToTI(WTextInput *ti)
{
    connect(ti, &WTextInput::enabled, this, &WInputMethodHelper::handleTIEnabled, Qt::UniqueConnection);
    connect(ti, &WTextInput::disabled, this, &WInputMethodHelper::handleTIDisabled, Qt::UniqueConnection);
    connect(ti, &WTextInput::requestLeave, ti, &WTextInput::sendLeave, Qt::UniqueConnection);
}

void WInputMethodHelper::disableTI(WTextInput *ti)
{
    Q_ASSERT(ti);
    W_D(WInputMethodHelper);
    if (focusedTextInput() == ti) {
        // Should we consider the case when the same text input is disabled and then enabled at the same time.
        auto im = inputMethod();
        if (im) {
            im->sendDeactivate();
            im->sendDone();
        }
        setFocusedTextInput(nullptr);
    }
}

void WInputMethodHelper::handleNewTI(WTextInput *ti)
{
    W_D(WInputMethodHelper);
    if (d->textInputs.contains(ti))
        return;
    d->textInputs.append(ti);
    connect(ti, &WTextInput::entityAboutToDestroy, this, [this, d, ti]{
        d->textInputs.removeAll(ti);
        disableTI(ti);
        ti->disconnect();
    }); // textInputs only save and traverse text inputs, do not handle connections
    // Whether this text input belongs to current seat or not, we should connect
    // its requestFocus signal for it might request focus from another seat to activate
    // itself here. For example, text input v1.
    connect(ti, &WTextInput::requestFocus, this, [this, ti, d]{
        if (ti->seat() && d->seat->name() == ti->seat()->name()) {
            connectToTI(ti);
            if (auto surface = d->seat->keyboardFocusSurface()) {
                ti->sendEnter(surface);
            }
        }
    });
    if (ti->seat() && d->seat->name() == ti->seat()->name()) {
        connectToTI(ti);
    }
}

void WInputMethodHelper::handleTIEnabled()
{
    WTextInput *ti = qobject_cast<WTextInput*>(sender());
    Q_ASSERT(ti);
    W_D(WInputMethodHelper);
    auto im = inputMethod();
    auto activeTI = focusedTextInput();
    if (activeTI == ti)
        return;
    if (activeTI) {
        if (im) {
            // If current active input method is not null, notify it to deactivate.
            im->sendDeactivate();
            im->sendDone();
        }
        // Notify last active text input to leave.
        activeTI->sendLeave();
    }
    setFocusedTextInput(ti);
    // Try to activate input method.
    if (im) {
        im->sendActivate();
        im->sendDone();
    }
}

void WInputMethodHelper::handleTIDisabled()
{
    WTextInput *ti = qobject_cast<WTextInput*>(sender());
    disableTI(ti);
}

void WInputMethodHelper::handleFocusedTICommitted()
{
    auto ti = focusedTextInput();
    Q_ASSERT(ti);
    auto im = inputMethod();
    if (im) {
        IME::Features features = ti->features();
        if (features.testFlag(IME::F_SurroundingText)) {
            im->sendSurroundingText(ti->surroundingText(), ti->surroundingCursor(), ti->surroundingAnchor());
        }
        im->sendTextChangeCause(ti->textChangeCause());
        if (features.testFlag(IME::F_ContentType)) {
            im->sendContentType(ti->contentHints().toInt(), ti->contentPurpose());
        }
        im->sendDone();
    }
    updateAllPopupSurfaces(ti->cursorRect());
}

void WInputMethodHelper::handleIMCommitted()
{
    auto im = inputMethod();
    Q_ASSERT(im);
    auto ti = focusedTextInput();
    if (ti) {
        ti->handleIMCommitted(im);
    }
}

void WInputMethodHelper::notifyLeave()
{
    if (auto ti = focusedTextInput()) {
        ti->sendLeave();
    }
}

void WInputMethodHelper::updateAllPopupSurfaces(QRect cursorRect)
{
    for (auto popup : d_func()->popupSurfaces) {
        updatePopupSurface(popup, cursorRect);
    }
}

void WInputMethodHelper::updatePopupSurface(WInputPopupSurface *popup, QRect cursorRect)
{
    Q_ASSERT(popup->handle());
    popup->sendCursorRect(cursorRect);
}

WAYLIB_SERVER_END_NAMESPACE
