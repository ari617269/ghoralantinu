import { createSlice } from '@reduxjs/toolkit';
const initialState = {
    token: null,
    user: null,
    isAuthenticated: false,
};
const authSlice = createSlice({
    name: 'auth',
    initialState,
    reducers: {
        setCredentials: (state, action) => {
            state.token = action.payload.token;
            state.user = action.payload.user;
            state.isAuthenticated = true;
        },
        clearCredentials: (state) => {
            state.token = null;
            state.user = null;
            state.isAuthenticated = false;
        },
    },
});
export const { setCredentials, clearCredentials } = authSlice.actions;
export default authSlice.reducer;
