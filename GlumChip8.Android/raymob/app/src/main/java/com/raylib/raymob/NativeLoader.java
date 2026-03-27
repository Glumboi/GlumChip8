/*
 *  raymob License (MIT)
 *
 *  Copyright (c) 2023-2024 Le Juez Victor
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

package com.raylib.raymob;

import android.app.NativeActivity;
import android.view.KeyEvent;
import android.os.Bundle;
import android.content.Intent;
import android.net.Uri;
import java.io.InputStream;
import java.io.IOException;

public class NativeLoader extends NativeActivity {

    // Required default no-arg constructor to satisfy Lint
    public NativeLoader() {
        super();
    }

    public DisplayManager displayManager;
    public SoftKeyboard softKeyboard;
    public boolean initCallback = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        displayManager = new DisplayManager(this);
        softKeyboard = new SoftKeyboard(this);

        // Load native library
        System.loadLibrary("raymob");

        // Request file before starting emulation
        openFilePicker();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (BuildConfig.FEATURE_DISPLAY_IMMERSIVE && hasFocus) {
            displayManager.setImmersiveMode();
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        softKeyboard.onKeyUpEvent(event);
        return super.onKeyDown(keyCode, event);
    }

    /**
     * Opens Android file picker to select a ROM before running Chip8.
     */
    public void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*"); // Can restrict to ".ch8"
        startActivityForResult(intent, 1); // 1 = requestCode
    }

    /**
     * Called when the user selects a file. Sends the URI string to native code.
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();
            try (InputStream is = getContentResolver().openInputStream(uri)) {
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                sendBytesToNative(buffer); // send to C
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    // JNI methods
    public native void sendBytesToNative(byte[] buffer);

    private native void sendUriToNative(String uri);

    @Override
    protected void onStart() {
        super.onStart();
        if (initCallback)
            onAppStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (initCallback)
            onAppResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (initCallback)
            onAppPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (initCallback)
            onAppStop();
    }

    // Native lifecycle callbacks
    private native void onAppStart();

    private native void onAppResume();

    private native void onAppPause();

    private native void onAppStop();
}