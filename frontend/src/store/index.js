import { configureStore } from '@reduxjs/toolkit';
import authReducer from './authSlice';
const store = configureStore({
    reducer: {
        auth: authReducer,
    },
});
export const useAppDispatch = () => store.dispatch;
export const useAppSelector = (selector) => selector(store.getState());
export default store;
