package org.kde.necessitas.industrius;

import android.app.Activity;

public class QtNativeLibrariesDir {
    public static String nativeLibrariesDir(Activity activity)
    {
        String m_nativeLibraryDir = null;
        m_nativeLibraryDir = "/data/data/"+activity.getPackageName()+"/lib/";
        return m_nativeLibraryDir;
    }
}
