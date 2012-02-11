/* Copyright (C) 2005-2011, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Mumble Developers nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioStats.h"

#include <QPainter>

namespace Mumble
{
    AudioBar::AudioBar(QWidget *p) : 
        QWidget(p) 
    {
	    qcBelow = Qt::yellow;
	    qcAbove = Qt::red;
	    qcInside = Qt::green;

	    iMin = 0;
	    iMax = 32768;
	    iBelow = 2000;
	    iAbove = 22000;
	    iValue = 1000;
	    iPeak = -1;
	    setMinimumSize(100,20);

	    qlReplacableColors	<< Qt::yellow << Qt::red << Qt::green << Qt::blue;
	    qlReplacementBrushes << Qt::BDiagPattern << Qt::DiagCrossPattern << Qt::NoBrush	<< Qt::FDiagPattern;
    }

    void AudioBar::paintEvent(QPaintEvent *)
    {
	    QPainter p(this);

	    if (isEnabled()) 
        {
		    qcBelow.setAlphaF(1.0f);
		    qcAbove.setAlphaF(1.0f);
		    qcInside.setAlphaF(1.0f);
	    } 
        else 
        {
		    qcBelow.setAlphaF(0.5f);
		    qcAbove.setAlphaF(0.5f);
		    qcInside.setAlphaF(0.5f);
	    }

	    if (iBelow > iAbove)
		    iBelow = iAbove;

	    if (iValue < iMin)
		    iValue = iMin;
	    else if (iValue > iMax)
		    iValue = iMax;

	    float scale = static_cast<float>(width()) / static_cast<float>(iMax - iMin);
	    int h = height();

	    int val = static_cast<int>(static_cast<float>(iValue) * scale + 0.5f);
	    int below = static_cast<int>(static_cast<float>(iBelow) * scale + 0.5f);
	    int above = static_cast<int>(static_cast<float>(iAbove) * scale  + 0.5f);
	    int max = static_cast<int>(static_cast<float>(iMax) * scale + 0.5f);
	    int min = static_cast<int>(static_cast<float>(iMin) * scale + 0.5f);
	    int peak = static_cast<int>(static_cast<float>(iPeak) * scale + 0.5f);


	    if (val <= below) 
        {
		    p.fillRect(0, 0, val, h, qcBelow);
		    p.fillRect(val, 0, below-val, h, qcBelow.darker(300));
		    p.fillRect(below, 0, above-below, h, qcInside.darker(300));
		    p.fillRect(above, 0, max-above, h, qcAbove.darker(300));
	    } 
        else if (val <= above) 
        {
		    p.fillRect(0, 0, below, h, qcBelow);
		    p.fillRect(below, 0, val-below, h, qcInside);
		    p.fillRect(val, 0, above-val, h, qcInside.darker(300));
		    p.fillRect(above, 0, max-above, h, qcAbove.darker(300));
	    } 
        else 
        {
		    p.fillRect(0, 0, below, h, qcBelow);
		    p.fillRect(below, 0, above-below, h, qcInside);
		    p.fillRect(above, 0, val-above, h, qcAbove);
		    p.fillRect(val, 0, max-val, h, qcAbove.darker(300));
	    }

	    if ((peak >= min) && (peak <= max))
        {
		    if (peak <= below)
			    p.setPen(qcBelow.lighter(150));
		    else if (peak <= above)
			    p.setPen(qcInside.lighter(150));
		    else
			    p.setPen(qcAbove.lighter(150));
		    p.drawLine(peak, 0, peak, h);
	    }
    }
}