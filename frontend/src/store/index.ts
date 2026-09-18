import { configureStore } from '@reduxjs/toolkit';
import authReducer from './authSlice';
import projectReducer from './projectSlice';

const store = configureStore({
  reducer: {
    auth: authReducer,
    project: projectReducer,
  },
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;

export const useAppDispatch = () => store.dispatch;
export const useAppSelector = <T,>(selector: (state: RootState) => T): T => selector(store.getState());

export default store;
