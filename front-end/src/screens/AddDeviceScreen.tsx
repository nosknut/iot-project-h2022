import React, { useState } from 'react'
import { Button, FormControl, Grid, InputLabel, MenuItem, Select, TextField, Typography } from '@mui/material';
import { API, graphqlOperation } from 'aws-amplify';
import { useNavigate, useSearchParams } from 'react-router-dom';
import { createDevice } from '../graphql/mutations'
import { v4 as uuidv4 } from 'uuid';

export enum DeviceTypes {
    TEMP_SENSOR = 'TEMP_SENSOR',
}

export const DeviceTypeNames: { [type: string]: string } = {
    [DeviceTypes.TEMP_SENSOR]: 'Temp Sensor'
}

export const AddDeviceScreen = () => {
    const [searchParams] = useSearchParams()
    const navigate = useNavigate()

    const [deviceId, setDeviceId] = useState(searchParams.get('deviceId'));
    const [name, setName] = useState('');
    const [deviceType, setDeviceType] = useState<DeviceTypes>(DeviceTypes.TEMP_SENSOR);

    const createNewDevice = async () => {
        const id = uuidv4()
        await API.graphql(graphqlOperation(createDevice, {
            input: {
                id,
                deviceId,
                name,
                type: deviceType,
            }
        }))
        navigate("/")
    }

    return (
        <Grid container height="100%" marginX="auto" justifyItems="center" maxWidth="md" padding={1}>

            <Grid item xs={12} paddingY={1}>
                <Typography variant="body1" align="center">
                    Scan the QR code on your device packaging with your phone camera to add it to your account.
                </Typography>
            </Grid>
            <Grid item xs={12} paddingY={1}>
                <TextField fullWidth label="Device Id" value={deviceId} onChange={(e) => setDeviceId(e.target.value)} />
            </Grid>
            <Grid item xs={12} paddingY={1}>
                <TextField fullWidth label="Device Name" value={name} onChange={(e) => setName(e.target.value)} />
            </Grid>
            <Grid item xs={12} paddingY={1}>
                <FormControl fullWidth>
                    <InputLabel id="device-type-select-label">Device Type</InputLabel>
                    <Select
                        labelId="device-type-select-label"
                        id="device-type-select"
                        value={deviceType || ""}
                        label="Device Type"
                        onChange={(e) => setDeviceType(e.target.value as DeviceTypes)}
                    >
                        <MenuItem value={DeviceTypes.TEMP_SENSOR}>{DeviceTypeNames[DeviceTypes.TEMP_SENSOR]}</MenuItem>
                    </Select>
                </FormControl>
            </Grid>
            <Grid item xs={12} paddingY={1}>
                <Button
                    fullWidth
                    variant="contained"
                    onClick={() => createNewDevice().catch(console.error)}
                    disabled={!name || !deviceType || !deviceId}
                >Add Device</Button>
            </Grid>
        </Grid >
    )
}
