import { SignOut } from '@aws-amplify/ui-react/dist/types/components/Authenticator/Authenticator';
import { Logout, ArrowBack } from '@mui/icons-material';
import { Box, IconButton, Toolbar, Typography } from '@mui/material';
import AppBar from '@mui/material/AppBar/AppBar';
import React from 'react';
import { Route, Routes, useNavigate } from 'react-router';
import { ThemeButton } from '../components/Themes';
import { InstallAppButton } from './InstallAppButton';

type Props = {
    signOut?: SignOut
}

export function PageNavigation({ signOut }: Props) {
    const navigate = useNavigate()
    return (
        <AppBar title="Temp Dashboard">
            <Toolbar>
                <Routes>
                    <Route path="/devices/add" element={
                        <IconButton sx={{ color: 'white' }} onClick={() => navigate("/")}>
                            <ArrowBack />
                        </IconButton>
                    } />
                </Routes>
                <Typography variant="h6" color="inherit">
                    Temp Dashboard
                </Typography>
                <Box marginLeft="auto" />
                <InstallAppButton />
                <ThemeButton />
                <IconButton sx={{ color: 'white' }} edge="end" onClick={signOut}>
                    <Logout />
                </IconButton>
            </Toolbar>
        </AppBar>
    )
}
