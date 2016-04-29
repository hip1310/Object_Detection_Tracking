package com.example.trackmf;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Mat;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.MediaRecorder;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.Toast;

public class MainActivity extends Activity implements CvCameraViewListener2 {

	private static final String    TAG = "com.example.trackmf.MainActivity";
	
	private CameraBridgeViewBase   mOpenCvCameraView;
	private Mat mRgba;
	private Mat mGray;
	
	private MenuItem               mItemCam;
    private MenuItem               mItemRes1280;
    private MenuItem               mItemRes640;
    private MenuItem               mItemRes1920;
    private MenuItem               mItemRec;
    
    boolean camera = true;
    
	private static final int PERMISSION_CODE = 1;
    private int mScreenDensity;
    private MediaProjectionManager mProjectionManager;
    private static final int DISPLAY_WIDTH = 640;
    private static final int DISPLAY_HEIGHT = 480;
    private MediaProjection mMediaProjection;
    private VirtualDisplay mVirtualDisplay;
    private MediaProjectionCallback mMediaProjectionCallback;
    private MediaRecorder mMediaRecorder;
    private boolean recording = false;
	
	private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");
                    
                 // Load native library after(!) OpenCV initialization
                    System.loadLibrary("tracker");
                    System.loadLibrary("opencv_java3");
                    
                    load();
                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };
    
    @Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(R.layout.activity_main);
		
		DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        mScreenDensity = metrics.densityDpi;
        
		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.cameraView);
		mOpenCvCameraView.setMaxFrameSize(640, 480);
		mOpenCvCameraView.setCvCameraViewListener(this);
		
		mMediaRecorder = new MediaRecorder();
        mProjectionManager = (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        mMediaProjectionCallback = new MediaProjectionCallback();
	}
	
	@Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    public void onDestroy() {
        super.onDestroy();
        mOpenCvCameraView.disableView();
        if (mMediaProjection != null) {
            mMediaProjection.stop();
            mMediaProjection = null;
        }
    }

    public void onCameraViewStarted(int width, int height) {
        mGray = new Mat();
        mRgba = new Mat();
    }

    public void onCameraViewStopped() {
        mGray.release();
        mRgba.release();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
    	mRgba = inputFrame.rgba();
    	
    	detectMotion(mRgba.getNativeObjAddr());
    	
    	return mRgba;
    }

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		mItemCam = menu.add("Camera");
        mItemRes1920 = menu.add("1920*1080");
        mItemRes1280 = menu.add("1280*960");
        mItemRes640 = menu.add("640*480");
        mItemRec = menu.add("Record");
  
		return true;
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
	    if(recording) {
	        mItemRec.setTitle("Stop");
	    }
	    else{
	    	mItemRec.setTitle("Record");
	    }
	    return super.onPrepareOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		if (item == mItemCam){
			Toast.makeText(getApplicationContext(), "Camera Change", Toast.LENGTH_SHORT).show();
			
			if(camera == true){
				mOpenCvCameraView.setCameraIndex(1);
				mOpenCvCameraView.disableView();
				mOpenCvCameraView.enableView();
				camera = false;
			}
			else{
				mOpenCvCameraView.setCameraIndex(-1);
				mOpenCvCameraView.disableView();
				mOpenCvCameraView.enableView();
				camera = true;
			}
		}    
        else if (item == mItemRes1920){
            mOpenCvCameraView.setMaxFrameSize(1920,1080);
			mOpenCvCameraView.disableView();
			mOpenCvCameraView.enableView();
        }	
        else if (item == mItemRes1280){
            mOpenCvCameraView.setMaxFrameSize(1280,960);
            mOpenCvCameraView.disableView();
			mOpenCvCameraView.enableView();
        }	
        else if (item == mItemRes640){
            mOpenCvCameraView.setMaxFrameSize(640,480);
            mOpenCvCameraView.disableView();
			mOpenCvCameraView.enableView();
        }
        else if (item == mItemRec){
            if (!recording) {
            	recording = true;
            	invalidateOptionsMenu();
                initRecorder();
                prepareRecorder();
                shareScreen();
            } else {
            	recording = false;
            	invalidateOptionsMenu();
            	mMediaRecorder.stop();
                mMediaRecorder.reset();
                Log.v(TAG, "Recording Stopped");
                stopScreenSharing();
                //initRecorder();
                //prepareRecorder();
            }
        }
		
        return true;
	}
	
	
	    @Override
	    public void onActivityResult(int requestCode, int resultCode, Intent data) {
	        if (requestCode != PERMISSION_CODE) {
	            Log.e(TAG, "Unknown request code: " + requestCode);
	            return;
	        }
	        if (resultCode != RESULT_OK) {
	            Toast.makeText(this,"Screen Cast Permission Denied", Toast.LENGTH_SHORT).show();
	            return;
	        }
	        mMediaProjection = mProjectionManager.getMediaProjection(resultCode, data);
	        mMediaProjection.registerCallback(mMediaProjectionCallback, null);
	        mVirtualDisplay = createVirtualDisplay();
	        mMediaRecorder.start();
	    }
	 
	 
	    private void shareScreen() {
	        if (mMediaProjection == null) {
	            startActivityForResult(mProjectionManager.createScreenCaptureIntent(), PERMISSION_CODE);
	            return;
	        }
	        mVirtualDisplay = createVirtualDisplay();
	        mMediaRecorder.start();
	    }
	 
	    private void stopScreenSharing() {
	        if (mVirtualDisplay == null) {
	            return;
	        }
	        mVirtualDisplay.release();
	        //mMediaRecorder.release();
	    }
	 
	    private VirtualDisplay createVirtualDisplay() {
	        return mMediaProjection.createVirtualDisplay("MainActivity",
	                DISPLAY_WIDTH, DISPLAY_HEIGHT, mScreenDensity,
	                DisplayManager.VIRTUAL_DISPLAY_FLAG_AUTO_MIRROR,
	                mMediaRecorder.getSurface(), null /*Callbacks*/, null /*Handler*/);
	    }
	 
	    private class MediaProjectionCallback extends MediaProjection.Callback {
	        @Override
	        public void onStop() {
	          
	                mMediaRecorder.stop();
	                mMediaRecorder.reset();
	                Log.v(TAG, "Recording Stopped");
	                initRecorder();
	                prepareRecorder();
	          
	                mMediaProjection = null;
	                stopScreenSharing();
	                Log.i(TAG, "MediaProjection Stopped");
	        }
	    }
	 
	    private void prepareRecorder() {
	        try {
	            mMediaRecorder.prepare();
	        } catch (IllegalStateException e) {
	            e.printStackTrace();
	            finish();
	        } catch (IOException e) {
	            e.printStackTrace();
	            finish();
	        }
	    }
	 
	    private void initRecorder() {
	       // mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
	        mMediaRecorder.setVideoSource(MediaRecorder.VideoSource.SURFACE);
	        mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
	        mMediaRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.H264);
	       // mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
	        mMediaRecorder.setVideoEncodingBitRate(512 * 1000);
	        mMediaRecorder.setVideoFrameRate(30);
	        mMediaRecorder.setVideoSize(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	        
	        SimpleDateFormat sdf = new SimpleDateFormat("MMddyy_HHmmss");
	        String currentDateandTime = sdf.format(new Date());
	        
	        mMediaRecorder.setOutputFile(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) + "capture" + currentDateandTime + ".mp4");
	    }
	public native void load();
	public native void detectMotion(long matAddrRgba);
}