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

import android.content.Context;
import android.text.InputType;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public class QtEditText extends View
{
    QtInputConnection m_inputConnection;
    int m_initialCapsMode = 0;
    int m_imeOptions = 0;
    int m_inputType = InputType.TYPE_CLASS_TEXT;

    public void setImeOptions(int m_imeOptions) {
        this.m_imeOptions = m_imeOptions;
    }

    public void setInitialCapsMode(int m_initialCapsMode) {
        this.m_initialCapsMode = m_initialCapsMode;
    }


    public void setInputType(int m_inputType) {
        this.m_inputType = m_inputType;
    }

    public QtEditText(Context context) {
        super(context);
        setFocusable(true);
        setFocusableInTouchMode(true);
        m_inputConnection = new QtInputConnection(this);
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        outAttrs.inputType = m_inputType;
        outAttrs.imeOptions = m_imeOptions;
        outAttrs.initialCapsMode = m_initialCapsMode;
        outAttrs.imeOptions |= EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        return m_inputConnection;
    }
// // DEBUG CODE
//    @Override
//    protected void onDraw(Canvas canvas) {
//        canvas.drawARGB(127, 255, 0, 255);
//        super.onDraw(canvas);
//    }
}
