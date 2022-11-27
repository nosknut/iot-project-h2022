import { GetApp } from "@mui/icons-material"
import { IconButton } from "@mui/material"

import { usePwaInstall } from "../hooks/usePwaInstalled"

export function InstallAppButton() {
    const {
        isAppInstalled,
        isAppInstallable,
        installApp,
    } = usePwaInstall()

    return (
        (isAppInstallable && !isAppInstalled) ? (
            <IconButton sx={{ color: 'white' }} onClick={installApp}>
                <GetApp />
            </IconButton>
        ) : null
    )
}