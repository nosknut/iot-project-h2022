import React from 'react';
import { Box, Grid, Typography } from '@mui/material';
import { useCumulativeTelemetry } from '../streams/cumulativeTelemetry';
import { LineChart, Line, CartesianGrid, YAxis, XAxis, Tooltip, ResponsiveContainer } from 'recharts';

export function RealtimePlot() {
    const cumulativeTelemetry = useCumulativeTelemetry();
    
    return (
        <>
            <Grid item xs={12} paddingY={1}>
                <Typography variant="h6" align="center">
                    Temperature
                </Typography>
            </Grid>
            <Grid item xs={12} paddingY={1}>
                <Typography variant="h6" align="center">
                    Door {cumulativeTelemetry[cumulativeTelemetry.length - 1]?.DoorOpen ? "Open" : "Closed"}
                </Typography>
            </Grid>
            <Grid item xs={12} paddingY={1}>
                <Box marginX="auto">
                    <ResponsiveContainer width="100%" height={300}>
                    <LineChart data={cumulativeTelemetry} margin={{ top: 5, right: 20, bottom: 5, left: 0 }}>
                        <Line type="monotone" dataKey="BottomBackLeft" stroke="red"/>
                        <Line type="monotone" dataKey="BottomBackRight" stroke="blue"/>
                        <Line type="monotone" dataKey="BottomFrontLeft" stroke="orange"/>
                        <Line type="monotone" dataKey="BottomFrontRight" stroke="green"/>
                        <Line type="monotone" dataKey="TopBackLeft" stroke="purple"/>
                        <Line type="monotone" dataKey="TopBackRight" stroke="pink"/>
                        <Line type="monotone" dataKey="TopFrontLeft" stroke="darkgreen"/>
                        <Line type="monotone" dataKey="TopFrontRight" stroke="darkblue"/>
                        <Line type="monotone" dataKey="Outside" stroke="darkred"/>
                        <CartesianGrid stroke="#ccc" strokeDasharray="5 5" />
                        <XAxis dataKey="Time" />
                        <YAxis />
                        <Tooltip />
                    </LineChart>
                    </ResponsiveContainer>
                </Box>
            </Grid>
        </>
    )
}
