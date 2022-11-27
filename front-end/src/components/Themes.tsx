import { Brightness6, BrightnessLow } from "@mui/icons-material";
import { createTheme, IconButton, ThemeOptions, ThemeProvider } from "@mui/material";
import useLocalStorage from "@rehooks/local-storage";
import { useMemo } from "react";

if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
    if (localStorage.getItem('theme') === null) {
        localStorage.setItem('theme', 'dark')
    }
}

const lightTheme: ThemeOptions = {
    palette: {
        mode: 'light',
    }
}

const darkTheme: ThemeOptions = {
    palette: {
        mode: 'dark',
    }
}

export function useDarkMode(): [boolean, (theme: 'light' | 'dark') => void] {
    const [theme, setTheme] = useLocalStorage('theme')
    const isDarkMode = theme === 'dark'
    return [isDarkMode, setTheme]
}

export function ThemeButton() {
    const [isDarkMode, setTheme] = useDarkMode()
    return (
        <IconButton id="change-theme-btn" sx={{ color: 'white' }} onClick={() => setTheme(isDarkMode ? 'light' : 'dark')}>
            {isDarkMode ? <Brightness6 /> : <BrightnessLow />}
        </IconButton>
    )
}

export function Themes({ children }: { children: React.ReactNode }) {
    const [isDarkMode] = useDarkMode()
    return (
        <ThemeProvider theme={useMemo(() => createTheme(isDarkMode ? darkTheme : lightTheme), [isDarkMode])}>
            {children}
        </ThemeProvider>
    )
}