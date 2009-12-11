/***************************************************************************
 *   Copyright (C) 2009 by Rick Helmus   *
 *   rhelmus_AT_gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QVBoxLayout>

#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "sensorplot.h"

CSensorPlot::CSensorPlot(const QString &title, QWidget *parent,
                         Qt::WindowFlags flags) : QWidget(parent, flags)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    
    hbox->addWidget(sensorPlot = new QwtPlot(title));
    sensorPlot->setAxisTitle(QwtPlot::xBottom, "time");
    sensorPlot->setAxisTitle(QwtPlot::yLeft, "ADC");
    
    hbox->addLayout(LCDLayout = new QVBoxLayout);
}

void CSensorPlot::addSensor(const std::string &name, const QColor &color)
{
    QwtPlotCurve *curve = new QwtPlotCurve(name.c_str());
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setPen(QPen(color));
    curve->attach(sensorPlot);

    sensorPlot->replot();

    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    LCDLayout->addWidget(frame);

    QVBoxLayout *vbox = new QVBoxLayout(frame);

    QLabel *label = new QLabel(name.c_str());
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);
    
    QLCDNumber *LCD = new QLCDNumber(frame);
    LCD->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    vbox->addWidget(LCD);

    sensorMap.insert(std::make_pair(name, SSensor(curve, LCD)));

    if ((sensorMap.size() > 1) && !sensorPlot->legend())
        sensorPlot->insertLegend(new QwtLegend, QwtPlot::RightLegend);
}

void CSensorPlot::addData(const std::string &name, const double x, const double y)
{
    TSensorMap::iterator it = sensorMap.find(name);

    if (it == sensorMap.end())
        return; // UNDONE: Error handling and stuff
    
    SSensor &sensor = it->second;
    
    // UNDONE: Size restraining
    sensor.xdata.push_back(x);
    sensor.ydata.push_back(y);

    sensor.sensorCurve->setRawData(&sensor.xdata[0], &sensor.ydata[0], sensor.xdata.size());

    // Update last value
    sensor.LCD->display(y);

    sensorPlot->replot();
}
