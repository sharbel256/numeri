import { Route, Routes } from "react-router-dom";

import { Layout } from "./components/Layout";
import Home from "./routes/Home";
import Releases from "./routes/Releases";

export default function App() {
  return (
    <Routes>
      <Route element={<Layout />}>
        <Route path="/" element={<Home />} />
        <Route path="/releases" element={<Releases />} />
      </Route>
    </Routes>
  );
}
