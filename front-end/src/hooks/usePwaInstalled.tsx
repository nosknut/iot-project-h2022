import { bind } from '@react-rxjs/core';
import { useCallback } from 'react';
import { fromEvent, map } from 'rxjs';

const beforeinstallprompt$ = fromEvent(window, 'beforeinstallprompt')
    .pipe(map((event: any) => ({
        deferredPrompt: event,
        isAppInstallable: true,
    })))

const [useBeforeInstallPrompt] = bind(beforeinstallprompt$, {
    deferredPrompt: null,
    isAppInstallable: false,
})

const isAppInstalled$ = fromEvent(window, 'appinstalled').pipe(map(() => true))

const [useIsAppInstalled] = bind(isAppInstalled$, false)

export function usePwaInstall() {
    const {
        deferredPrompt,
        isAppInstallable,
    } = useBeforeInstallPrompt()
    const isAppInstalled = useIsAppInstalled()

    return {
        isAppInstallable,
        isAppInstalled,
        installApp: useCallback(() => {
            if (deferredPrompt) {
                deferredPrompt?.prompt()
            }
        }, [deferredPrompt]),
    }
}