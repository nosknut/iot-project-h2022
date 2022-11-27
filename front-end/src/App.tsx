import React from 'react'
import { Amplify } from 'aws-amplify'

import { withAuthenticator, WithAuthenticatorProps } from '@aws-amplify/ui-react';
import '@aws-amplify/ui-react/styles.css';
import { BrowserRouter, Route, Routes } from 'react-router-dom';

import awsExports from "./aws-exports";
import { PageNavigation } from './components/PageNavigation';
import { Dashboard } from './screens/Dashboard';
import { AddDeviceScreen } from './screens/AddDeviceScreen';
import { useTheme } from '@mui/material';

Amplify.configure(awsExports);

const App = ({ signOut }: WithAuthenticatorProps) => {
  const theme = useTheme()

  return (
    <BrowserRouter>
      <PageNavigation signOut={signOut} />
      <main style={{ width: '100%', height: '100%', paddingTop: theme.spacing(10) }}>
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/devices/add" element={<AddDeviceScreen />} />
        </Routes>
      </main>
    </BrowserRouter>
  )
}

export default withAuthenticator(App);
