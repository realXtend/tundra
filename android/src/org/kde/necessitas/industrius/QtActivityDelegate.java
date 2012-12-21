/*
    Copyright (c) 2009-2011, BogDan Vatra <bog_dan_ro@yahoo.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the  BogDan Vatra <bog_dan_ro@yahoo.com> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY BogDan Vatra <bog_dan_ro@yahoo.com> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL BogDan Vatra <bog_dan_ro@yahoo.com> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package org.kde.necessitas.industrius;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.method.MetaKeyKeyListener;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

public class QtActivityDelegate
{
    private Activity m_activity = null;
    private Method m_super_onRestoreInstanceState = null;
    private Method m_super_onRetainNonConfigurationInstance = null;
    private Method m_super_onSaveInstanceState = null;
    private Method m_super_onKeyDown = null;
    private Method m_super_onKeyUp = null;
    private Method m_super_dispatchKeyEvent = null;
    private Method m_super_onCreateOptionsMenu = null;
    private Method m_super_onPrepareOptionsMenu = null;
    private Method m_super_onOptionsItemSelected = null;

    private static final String NATIVE_LIBRARIES_KEY="native.libraries";
    private static final String BUNDLED_LIBRARIES_KEY="bundled.libraries";
    private static final String MAIN_LIBRARY_KEY="main.library";
    private static final String ENVIRONMENT_VARIABLES_KEY="environment.variables";
    private static final String APPLICATION_PARAMETERS_KEY="application.parameters";
    private static final String STATIC_INIT_CLASSES_KEY="static.init.classes";
    private static final String NECESSITAS_API_LEVEL_KEY="necessitas.api.level";

    private static String m_environmentVariables = null;
    private static String m_applicationParameters = null;


    private String m_mainLib;
    private long m_metaState;
    private int m_lastChar=0;
    private boolean m_fullScreen = false;
    private boolean m_started = false;
    private QtSurface m_surface = null;
    private QtLayout m_layout = null;
    private QtEditText m_editText = null;
    private InputMethodManager m_imm = null;
    private boolean m_quitApp = true;
    private Process m_debuggerProcess=null; // debugger process

    public boolean m_keyboardIsVisible=false;
    public boolean m_keyboardIsHiding=false;

    public QtLayout getQtLayout()
    {
        return m_layout;
    }

    public QtSurface getQtSurface()
    {
        return m_surface;
    }

    public void redrawWindow(int left, int top, int right, int bottom) {
        m_surface.drawBitmap(new Rect(left, top, right, bottom));
    }

    public void setFullScreen(boolean enterFullScreen)
    {
        if (m_fullScreen == enterFullScreen)
            return;
        if (m_fullScreen = enterFullScreen)
        {
            m_activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            m_activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
        }
        else
        {
            m_activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
            m_activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
    }
    // case status
    private final int ImhNoAutoUppercase=0x2;
    private final int ImhPreferUppercase=0x8;
    private final int ImhPreferLowercase=0x10;
    private final int ImhUppercaseOnly=0x40000;
    private final int ImhLowercaseOnly=0x80000;

    // options
    private final int ImhNoPredictiveText=0x20;

    // layout
    private final int ImhHiddenText=0x1;
    private final int ImhPreferNumbers=0x4;
    private final int ImhDigitsOnly=0x10000;
    private final int ImhFormattedNumbersOnly=0x20000;
    private final int ImhDialableCharactersOnly=0x100000;
    private final int ImhEmailCharactersOnly=0x200000;
    private final int ImhUrlCharactersOnly=0x400000;

    public void resetSoftwareKeyboard()
    {
        if (m_imm == null)
            return;
        m_editText.postDelayed(new Runnable() {
            @Override
            public void run() {
                m_imm.restartInput(m_editText);
            }
        }, 5);
    }


    public void showSoftwareKeyboard(int x, int y, int width, int height, int inputHints)
    {
        if (m_imm == null)
            return;
        if (height>m_surface.getHeight()*2/3)
            m_activity.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_UNCHANGED | WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        else
            m_activity.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_UNCHANGED | WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN);

        int initialCapsMode = 0;
        int imeOptions = android.view.inputmethod.EditorInfo.IME_ACTION_DONE;
        int inputType = android.text.InputType.TYPE_CLASS_TEXT;
        if ( ((inputHints & ImhNoAutoUppercase) !=0 || (inputHints & ImhPreferUppercase) !=0 )
                && (inputHints & ImhLowercaseOnly) ==0 )
            initialCapsMode = android.text.TextUtils.CAP_MODE_SENTENCES;

        if ( (inputHints & ImhUppercaseOnly) != 0 )
            initialCapsMode = android.text.TextUtils.CAP_MODE_CHARACTERS;

        if ( (inputHints & ImhHiddenText) != 0 )
            inputType = android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD;
        if ( (inputHints & ImhPreferNumbers) != 0 )
            inputType = android.text.InputType.TYPE_CLASS_NUMBER;
        if ( (inputHints & ImhDigitsOnly) != 0 )
            inputType = android.text.InputType.TYPE_CLASS_NUMBER;
        if ( (inputHints & ImhFormattedNumbersOnly) != 0 )
            inputType = android.text.InputType.TYPE_CLASS_NUMBER
                        |android.text.InputType.TYPE_NUMBER_FLAG_DECIMAL
                        |android.text.InputType.TYPE_NUMBER_FLAG_SIGNED;
        if ( (inputHints & ImhDialableCharactersOnly) != 0 )
            inputType = android.text.InputType.TYPE_CLASS_PHONE;
        if ( (inputHints & ImhEmailCharactersOnly) != 0 )
            inputType = android.text.InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
        if ( (inputHints & ImhUrlCharactersOnly) != 0 )
        {
            inputType = android.text.InputType.TYPE_TEXT_VARIATION_URI;
            imeOptions = android.view.inputmethod.EditorInfo.IME_ACTION_GO;
        }

        if ( (inputHints & ImhNoPredictiveText) != 0 )
        {
            //android.text.InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS | android.text.InputType.TYPE_CLASS_TEXT;
            inputType = android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD;
        }

        m_editText.setInitialCapsMode(initialCapsMode);
        m_editText.setImeOptions(imeOptions);
        m_editText.setInputType(inputType);

        m_layout.removeView(m_editText);
        m_layout.addView(m_editText, new QtLayout.LayoutParams(width, height, x, y));
        m_editText.bringToFront();
        m_editText.requestFocus();
        m_editText.postDelayed(new Runnable() {
            @Override
            public void run() {
                m_imm.showSoftInput(m_editText, 0);
                m_keyboardIsVisible=true;
                m_keyboardIsHiding=false;
                m_editText.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        m_imm.restartInput(m_editText);
                    }
                }, 25);
            }
        }, 15);
    }

    public void hideSoftwareKeyboard()
    {
        if (m_imm == null)
            return;
        m_imm.hideSoftInputFromWindow(m_editText.getWindowToken(), 0);
        m_keyboardIsVisible=false;
        m_keyboardIsHiding=false;
    }

    public boolean loadApplication(Activity activity, ClassLoader classLoader, Bundle loaderParams)
    {
        /// check parameters integrity
        if (!loaderParams.containsKey(NATIVE_LIBRARIES_KEY)
                || !loaderParams.containsKey(BUNDLED_LIBRARIES_KEY)
                || !loaderParams.containsKey(ENVIRONMENT_VARIABLES_KEY)
                || !loaderParams.containsKey(APPLICATION_PARAMETERS_KEY))
            return false;

        m_activity = activity;
        QtNative.setActivity(m_activity, this);
        QtNative.setClassLoader(classLoader);
        if (loaderParams.containsKey(STATIC_INIT_CLASSES_KEY))
            for(String className: loaderParams.getStringArrayList(STATIC_INIT_CLASSES_KEY))
            {
                try {
                    @SuppressWarnings("rawtypes")
                    Class initClass = classLoader.loadClass(className);
                    Object staticInitDataObject=initClass.newInstance(); // create an instance
                    @SuppressWarnings("unchecked")
                    Method m = initClass.getMethod("setActivity", Activity.class, Object.class);
                    m.invoke(staticInitDataObject, m_activity, this);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

        QtNative.loadQtLibraries(loaderParams.getStringArrayList(NATIVE_LIBRARIES_KEY));
        ArrayList<String> libraries = loaderParams.getStringArrayList(BUNDLED_LIBRARIES_KEY);
        QtNative.loadBundledLibraries(libraries, QtNativeLibrariesDir.nativeLibrariesDir(m_activity));
        m_mainLib = loaderParams.getString(MAIN_LIBRARY_KEY);
        // older apps provide the main library as the last bundled library; look for this if the main library isn't provided
        if (null == m_mainLib && libraries.size()>0)
            m_mainLib = libraries.get(libraries.size()-1);

        try {
            m_super_onRestoreInstanceState = m_activity.getClass().getMethod("super_onRestoreInstanceState", Bundle.class);
            m_super_onRetainNonConfigurationInstance = m_activity.getClass().getMethod("super_onRetainNonConfigurationInstance");
            m_super_onSaveInstanceState = m_activity.getClass().getMethod("super_onSaveInstanceState", Bundle.class);
            m_super_onKeyDown = m_activity.getClass().getMethod("super_onKeyDown", Integer.TYPE, KeyEvent.class);
            m_super_onKeyUp = m_activity.getClass().getMethod("super_onKeyUp", Integer.TYPE, KeyEvent.class);
            m_super_dispatchKeyEvent = m_activity.getClass().getMethod("super_dispatchKeyEvent", KeyEvent.class);
            m_super_onCreateOptionsMenu = m_activity.getClass().getMethod("super_onCreateOptionsMenu", Menu.class);
            m_super_onPrepareOptionsMenu = m_activity.getClass().getMethod("super_onPrepareOptionsMenu", Menu.class);
            m_super_onOptionsItemSelected = m_activity.getClass().getMethod("super_onOptionsItemSelected", MenuItem.class);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        int necessitasApiLevel=1;
        if (loaderParams.containsKey(NECESSITAS_API_LEVEL_KEY))
            necessitasApiLevel=loaderParams.getInt(NECESSITAS_API_LEVEL_KEY);

        m_environmentVariables=loaderParams.getString(ENVIRONMENT_VARIABLES_KEY);
        final String additionalEnvironmentVariables="NECESSITAS_API_LEVEL="+necessitasApiLevel+"\tHOME="+m_activity.getFilesDir().getAbsolutePath()+"\tTMPDIR="+m_activity.getFilesDir().getAbsolutePath();
        if (m_environmentVariables != null && m_environmentVariables.length()>0)
            m_environmentVariables=additionalEnvironmentVariables+"\t"+m_environmentVariables;
        else
            m_environmentVariables=additionalEnvironmentVariables;

        m_applicationParameters=loaderParams.getString(APPLICATION_PARAMETERS_KEY);
        return true;
    }

    public boolean startApplication()
    {
        // start application
        try
        {
            // FIXME turn on debuggable check
            // if the applications is debuggable and it has a native debug request
            if ( /*(ai.flags&ApplicationInfo.FLAG_DEBUGGABLE) != 0
                    &&*/ m_activity.getIntent().getExtras() != null
                    && m_activity.getIntent().getExtras().containsKey("native_debug")
                    && m_activity.getIntent().getExtras().getString("native_debug").equals("true"))
            {
                try
                {
                    String packagePath=m_activity.getPackageManager().getApplicationInfo(m_activity.getPackageName(), PackageManager.GET_CONFIGURATIONS).dataDir+"/";
                    String gdbserverPath=m_activity.getIntent().getExtras().containsKey("gdbserver_path")?m_activity.getIntent().getExtras().getString("gdbserver_path"):packagePath+"lib/gdbserver ";
                    String socket=m_activity.getIntent().getExtras().containsKey("gdbserver_socket")?m_activity.getIntent().getExtras().getString("gdbserver_socket"):"+debug-socket";
                    // start debugger
                    m_debuggerProcess =Runtime.getRuntime().exec(gdbserverPath+socket+" --attach "+android.os.Process.myPid(),null, new File(packagePath));
                }
                catch (IOException ioe)
                {
                    Log.e(QtNative.QtTAG,"Can't start debugger"+ioe.getMessage());
                }
                catch (SecurityException se)
                {
                    Log.e(QtNative.QtTAG,"Can't start debugger"+se.getMessage());
                } catch (NameNotFoundException e) {
                    Log.e(QtNative.QtTAG,"Can't start debugger"+e.getMessage());
                }
            }

            if (/*(ai.flags&ApplicationInfo.FLAG_DEBUGGABLE) != 0
                    &&*/ m_activity.getIntent().getExtras() != null
                    && m_activity.getIntent().getExtras().containsKey("qml_debug")
                    && m_activity.getIntent().getExtras().getString("qml_debug").equals("true"))
            {
                String qmljsdebugger;
                if (m_activity.getIntent().getExtras().containsKey("qmljsdebugger")) {
                    qmljsdebugger = m_activity.getIntent().getExtras().getString("qmljsdebugger");
                    qmljsdebugger.replaceAll("\\s", ""); // remove whitespace for security
                } else {
                    qmljsdebugger = "port:3768";
                }
                m_applicationParameters += "\t-qmljsdebugger="+qmljsdebugger;
            }

            if (null == m_surface)
                onCreate(null);
            String nativeLibraryDir = QtNativeLibrariesDir.nativeLibrariesDir(m_activity);
            m_surface.applicationStarted( QtNative.startApplication(m_applicationParameters
                                                                , m_environmentVariables
                                                                , m_mainLib
                                                                , nativeLibraryDir));

            m_started = true;
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public void onTerminate()
    {
        QtNative.terminateQt();
    }

    public void onCreate(Bundle savedInstanceState)
    {
        m_quitApp = true;
        if (null == savedInstanceState)
        {
            DisplayMetrics metrics = new DisplayMetrics();
            m_activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
            QtNative.setApplicationDisplayMetrics(metrics.widthPixels, metrics.heightPixels,
                            metrics.widthPixels, metrics.heightPixels,
                            metrics.xdpi, metrics.ydpi);
        }
        m_layout=new QtLayout(m_activity);
        m_surface = new QtSurface(m_activity, 0);
        m_activity.registerForContextMenu(m_surface);
        m_editText = new QtEditText(m_activity);
        m_imm = (InputMethodManager)m_activity.getSystemService(Context.INPUT_METHOD_SERVICE);
        m_layout.addView(m_surface,0);
        m_activity.setContentView(m_layout
                , new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
                        ViewGroup.LayoutParams.FILL_PARENT));
        m_layout.bringChildToFront(m_surface);
    }


    public void onDestroy()
    {
        if (m_quitApp)
        {
            Log.i(QtNative.QtTAG, "onDestroy");
            if (m_debuggerProcess != null)
                m_debuggerProcess.destroy();
            System.exit(0);// FIXME remove it or find a better way
        }
    }

    public void onRestoreInstanceState(Bundle savedInstanceState)
    {
        try {
            m_super_onRestoreInstanceState.invoke(m_activity, savedInstanceState);
        } catch (Exception e) {
            e.printStackTrace();
        }
//        setFullScreen(savedInstanceState.getBoolean("FullScreen"));
        m_started = savedInstanceState.getBoolean("Started");
        if (m_started)
            m_surface.applicationStarted( true );
    }

    public void onResume()
    {
        // fire all lostActions
        synchronized (QtNative.m_mainActivityMutex)
        {
            Iterator<Runnable> itr=QtNative.getLostActions().iterator();
            while(itr.hasNext())
                m_activity.runOnUiThread(itr.next());
            if (m_started)
            {
                QtNative.clearLostActions();
                QtNative.updateWindow();
            }
        }
    }

    public Object onRetainNonConfigurationInstance()
    {
        try {
            m_super_onRetainNonConfigurationInstance.invoke(m_activity);
        } catch (Exception e) {
            e.printStackTrace();
        }
        m_quitApp = false;
        return true;
    }

    public void onSaveInstanceState(Bundle outState) {
        try {
            m_super_onSaveInstanceState.invoke(m_activity, outState);
        } catch (Exception e) {
            e.printStackTrace();
        }
        outState.putBoolean("FullScreen",m_fullScreen);
        outState.putBoolean("Started", m_started);
    }

    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        if (!m_started)
            return false;

        m_metaState = MetaKeyKeyListener.handleKeyDown(m_metaState, keyCode, event);
        int c = event.getUnicodeChar(MetaKeyKeyListener.getMetaState(m_metaState));
        int lc=c;
        m_metaState = MetaKeyKeyListener.adjustMetaAfterKeypress(m_metaState);

        if ((c & KeyCharacterMap.COMBINING_ACCENT) != 0)
        {
            c = c & KeyCharacterMap.COMBINING_ACCENT_MASK;
            int composed = KeyEvent.getDeadChar(m_lastChar, c);
            c = composed;
        }
        m_lastChar = lc;
        if (keyCode != KeyEvent.KEYCODE_BACK)
                QtNative.keyDown(keyCode, c, event.getMetaState());
        return true;
    }

    public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        if (!m_started)
            return false;

        if (keyCode == KeyEvent.KEYCODE_BACK && m_keyboardIsVisible)
        {
            if (!m_keyboardIsHiding)
                hideSoftwareKeyboard();
            return true;
        }

        m_metaState = MetaKeyKeyListener.handleKeyUp(m_metaState, keyCode, event);
        QtNative.keyUp(keyCode, event.getUnicodeChar(), event.getMetaState());
        return true;
    }

    public boolean dispatchKeyEvent(KeyEvent event)
    {
        if (m_started && event.getAction() == KeyEvent.ACTION_MULTIPLE &&
            event.getCharacters() != null &&
            event.getCharacters().length() == 1 &&
            event.getKeyCode() == 0)
        {
            QtNative.keyDown(0, event.getCharacters().charAt(0), event.getMetaState());
            QtNative.keyUp(0, event.getCharacters().charAt(0), event.getMetaState());
        }

        try {
            return (Boolean) m_super_dispatchKeyEvent.invoke(m_activity, event);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public boolean onCreateOptionsMenu(Menu menu)
    {
        QtNative.createOptionsMenu(menu);
        try {
            return (Boolean) m_super_onPrepareOptionsMenu.invoke(m_activity, menu);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean onPrepareOptionsMenu(Menu menu)
    {
        QtNative.prepareOptionsMenu(menu);
        try {
            return (Boolean) m_super_onPrepareOptionsMenu.invoke(m_activity, menu);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean onOptionsItemSelected(MenuItem item)
    {
        return QtNative.optionsItemSelected(item.getGroupId(), item.getItemId());
    }
}
