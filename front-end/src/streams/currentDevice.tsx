import { bind } from "@react-rxjs/core";
import { createSignal } from "@react-rxjs/utils";
import { Device } from "../API";

// Keeping these streams in a different file than the component will prevent the file from being reloaded
// when the component file is saved after being edited
// That way, the stream content will not be reset on save
export const [currentDevice$, setCurrentDevice] = createSignal<Device | null>()
export const [useCurrentDevice] = bind(currentDevice$, null)
