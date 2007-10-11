/***************************************************************************
 *   Copyright (C) 2007 by Shawn Starr <shawn.starr@rogers.com>            *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

/* Ion for Environment Canada XML data */

#ifndef _ION_ENVCAN_H_
#define _ION_ENVCAN_H_

#include <QtXml/QXmlStreamReader>
#include <QtCore/QStringList>
#include <QDebug>
#include <kurl.h>
#include <kio/job.h>
#include <kio/scheduler.h>
#include <kdemacros.h>
#include <plasma/dataengine.h>
#include "ion.h"
#include "weather_formula.h"

class WeatherData
{

public:
    // Warning info, can have more than one, especially in Canada, eh? :)
    struct WarningInfo {
        QString url;
        QString type;
        QString priority;
        QString description;
        QString timestamp;
    };

    // Five day forecast
    struct ForecastInfo {
        QString forecastPeriod;
        QString forecastSummary;
        QString shortForecast;

        QString forecastTempHigh;
        QString forecastTempLow;
        QString popPrecent;
        QString windForecast;

        QString precipForecast;
        QString precipType;
        QString precipTotalExpected;
        int forecastHumidity;
    };

    QString countryName;
    QString longTerritoryName;
    QString shortTerritoryName;
    QString cityName;
    QString regionName;
    QString stationID;

    // Current observation information.
    QString obsTimestamp;
    QString condition;
    QString temperature;
    QString dewpoint;

    // In winter windchill, in summer, humidex
    QString comforttemp;

    float pressure;
    QString pressureTendency;

    float visibility;
    QString humidity;

    QString windSpeed;
    QString windGust;
    QString windDirection;

    QVector <WeatherData::WarningInfo *> warnings;

    QString normalHigh;
    QString normalLow;

    QString forecastTimestamp;

    QString UVIndex;
    QString UVRating;

    // 5 day Forecast
    QVector <WeatherData::ForecastInfo *> forecasts;

    // Historical data from previous day.
    QString prevHigh;
    QString prevLow;
    QString prevPrecipType;
    QString prevPrecipTotal;

    // Almanac info
    QString sunriseTimestamp;
    QString sunsetTimestamp;
    QString moonriseTimestamp;
    QString moonsetTimestamp;

    // Historical Records
    float recordHigh;
    float recordLow;
    float recordRain;
    float recordSnow;
};

class KDE_EXPORT EnvCanadaIon : public IonInterface
{
    Q_OBJECT

public:
    EnvCanadaIon(QObject *parent, const QVariantList &args);
    ~EnvCanadaIon();
    void init();  // Setup the city location, fetching the correct URL name.
    void fetch(); // Get the City XML data.
    void updateData(void); // Sync data source with Applet
    void option(int option, QVariant value);

protected slots:
    void setup_slotDataArrived(KIO::Job *, const QByteArray &);
    void setup_slotJobFinished(KJob *);

    void slotDataArrived(KIO::Job *, const QByteArray &);
    void slotJobFinished(KJob *);

private:
    /* Environment Canada Methods - Internal for Ion */

    // Place information
    QString country(QString key);
    QString territory(QString key);
    QString city(QString key);
    QString region(QString key);
    QString station(QString key);

    // Current Conditions Weather info
    QString observationTime(QString key);
    QMap<QString, QString> warnings(QString key);
    QString condition(QString key);
    QMap<QString, QString> temperature(QString key);
    QString dewpoint(QString key);
    QString humidity(QString key);
    QString visibility(QString key);
    QMap<QString, QString> pressure(QString key);
    QMap<QString, QString> wind(QString key);
    QMap<QString, QString> regionalTemperatures(QString key);
    QMap<QString, QString> uvIndex(QString key);
    QVector<QString> forecasts(QString key);
    QMap<QString, QString> yesterdayWeather(QString key);
    QMap<QString, QString> sunriseSet(QString key);
    QMap<QString, QString> moonriseSet(QString key);
    QMap<QString, QString> weatherRecords(QString key);

    // Load and Parse the place XML listing
    void getXMLSetup();
    bool readXMLSetup();

    // Load and parse the specific place(s)
    void getXMLData();
    bool readXMLData(QString key, QXmlStreamReader& xml);

    // Check if place specified is valid or not
    bool validLocation(QString key);

    // Catchall for unknown XML tags
    void parseUnknownElement(QXmlStreamReader& xml);

    // Parse weather XML data
    WeatherData parseWeatherSite(WeatherData& data, QXmlStreamReader& xml);
    void parseDateTime(WeatherData& data, QXmlStreamReader& xml, WeatherData::WarningInfo* warning = NULL);
    void parseLocations(WeatherData& data, QXmlStreamReader& xml);
    void parseConditions(WeatherData& data, QXmlStreamReader& xml);
    void parseWarnings(WeatherData& data, QXmlStreamReader& xml);
    void parseWindInfo(WeatherData& data, QXmlStreamReader& xml);
    void parseWeatherForecast(WeatherData& data, QXmlStreamReader& xml);
    void parseRegionalNormals(WeatherData& data, QXmlStreamReader& xml);
    void parseForecast(WeatherData& data, QXmlStreamReader& xml, WeatherData::ForecastInfo* forecast);
    void parseShortForecast(WeatherData::ForecastInfo* forecast, QXmlStreamReader& xml);
    void parseForecastTemperatures(WeatherData::ForecastInfo* forecast, QXmlStreamReader& xml);
    void parseWindForecast(WeatherData::ForecastInfo* forecast, QXmlStreamReader& xml);
    void parsePrecipitationForecast(WeatherData::ForecastInfo* forecast, QXmlStreamReader& xml);
    void parsePrecipTotals(WeatherData::ForecastInfo* forecast, QXmlStreamReader& xml);
    void parseUVIndex(WeatherData& data, QXmlStreamReader& xml);
    void parseYesterdayWeather(WeatherData& data, QXmlStreamReader& xml);
    void parseAstronomicals(WeatherData& data, QXmlStreamReader& xml);
    void parseWeatherRecords(WeatherData& data, QXmlStreamReader& xml);

private:
    class Private;
    Private *const d;
};

K_EXPORT_PLASMA_ION(envcan, EnvCanadaIon)

#endif