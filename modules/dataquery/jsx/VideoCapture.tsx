import React from 'react';

import './dynamsoft.config';

// @ts-ignore
import type { CameraEnhancer as CE, CameraView as CV } from "dynamsoft-camera-enhancer";
// @ts-ignore
import type { CaptureVisionRouter as CVR } from "dynamsoft-capture-vision-router";
// @ts-ignore
import type { MultiFrameResultCrossFilter as MRCVF } from "dynamsoft-utility";
// @ts-ignore
import "dynamsoft-camera-enhancer";
// @ts-ignore
import "dynamsoft-capture-vision-router";
// @ts-ignore
import "dynamsoft-utility";

// HACK: For some reason, import from Dynamsoft does not work. But the library is creating a global
// `Dynamsoft` object that has all the library contents.
const Dynamsoft: any = (window as any).Dynamsoft;
const CameraEnhancer: typeof CE = Dynamsoft.DCE.CameraEnhancer;
const CameraView: typeof CV = Dynamsoft.DCE.CameraView;
const CaptureVisionRouter: typeof CVR = Dynamsoft.CVR.CaptureVisionRouter;
const MultiFrameResultCrossFilter: typeof MRCVF = Dynamsoft.Utility.MultiFrameResultCrossFilter;

const componentDestroyedErrorMsg = "VideoCapture Component Destroyed";

class VideoCapture extends React.Component {
  cameraViewContainer: React.RefObject<HTMLDivElement> = React.createRef();
  resultsContainer: React.RefObject<HTMLDivElement> = React.createRef();

  resolveInit?: () => void;
  pInit: Promise<void> = new Promise((r) => (this.resolveInit = r));
  isDestroyed = false;

  cvRouter?: CVR;
  cameraEnhancer?: CE;

  async componentDidMount() {
    try {
      // Create a `CameraEnhancer` instance for camera control and a `CameraView` instance for UI control.
      const cameraView = await CameraView.createInstance();
      if (this.isDestroyed) {
        throw Error(componentDestroyedErrorMsg);
      } // Check if component is destroyed after every async

      this.cameraEnhancer = await CameraEnhancer.createInstance(cameraView);
      if (this.isDestroyed) {
        throw Error(componentDestroyedErrorMsg);
      }

      // Get default UI and append it to DOM.
      this.cameraViewContainer.current!.append(cameraView.getUIElement());

      // Create a `CaptureVisionRouter` instance and set `CameraEnhancer` instance as its image source.
      this.cvRouter = await CaptureVisionRouter.createInstance();
      if (this.isDestroyed) {
        throw Error(componentDestroyedErrorMsg);
      }
      this.cvRouter.setInput(this.cameraEnhancer);

      // Define a callback for results.
      this.cvRouter.addResultReceiver({
        onDecodedBarcodesReceived: (result: any) => {
          if (!result.barcodeResultItems.length) return;

          this.resultsContainer.current!.textContent = "";
          console.log(result);
          for (let item of result.barcodeResultItems) {
            this.resultsContainer.current!.textContent += `${item.formatString}: ${item.text}\n\n`;
          }
        },
      });

      // Filter out unchecked and duplicate results.
      const filter = new MultiFrameResultCrossFilter();
      // Filter out unchecked barcodes.
      filter.enableResultCrossVerification("barcode", true);
      // Filter out duplicate barcodes within 3 seconds.
      filter.enableResultDeduplication("barcode", true);
      await this.cvRouter.addResultFilter(filter);
      if (this.isDestroyed) {
        throw Error(componentDestroyedErrorMsg);
      }

      // Open camera and start scanning single barcode.
      await this.cameraEnhancer.open();
      if (this.isDestroyed) {
        throw Error(componentDestroyedErrorMsg);
      }
      await this.cvRouter.startCapturing("ReadSingleBarcode");
      if (this.isDestroyed) {
        throw Error(componentDestroyedErrorMsg);
      }
    } catch (ex: any) {
      if ((ex as Error)?.message === componentDestroyedErrorMsg) {
        console.log(componentDestroyedErrorMsg);
      } else {
        let errMsg = ex.message || ex;
        console.error(errMsg);
        alert(errMsg);
      }
    }

    // Resolve pInit promise once initialization is complete.
    this.resolveInit!();
  }

  async componentWillUnmount() {
    this.isDestroyed = true;
    try {
      // Wait for the pInit to complete before disposing resources.
      await this.pInit;
      this.cvRouter?.dispose();
      this.cameraEnhancer?.dispose();
    } catch (_) {}
  }

  shouldComponentUpdate() {
    // Never update UI after mount, sdk use native way to bind event, update will remove it.
    return false;
  }

  render() {
    return (
      <div>
        <div ref={this.cameraViewContainer} style={{  width: "100%", height: "70vh" }}></div>
        <br />
        Results:
        <div ref={this.resultsContainer} className="results"></div>
      </div>
    );
  }
}

export default VideoCapture;
