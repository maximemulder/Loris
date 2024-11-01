import {createRoot} from 'react-dom/client';
import VideoCapture from './VideoCapture';

window.addEventListener('load', () => {
  const element = document.getElementById('lorisworkspace');
  if (!element) {
    throw new Error('Missing lorisworkspace');
  }
  const root = createRoot(element);

  root.render(
    <VideoCapture />,
  );
});

export default MyComponent;
