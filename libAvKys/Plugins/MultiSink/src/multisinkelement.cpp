/* Webcamoid, webcam capture application.
 * Copyright (C) 2016  Gonzalo Exequiel Pedone
 *
 * Webcamoid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Webcamoid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.
 *
 * Web-Site: http://webcamoid.github.io/
 */

#include <QSharedPointer>
#include <QQmlContext>
#include <akpacket.h>

#include "multisinkelement.h"
#include "multisinkelementsettings.h"
#include "mediawriter.h"
#include "multisinkutils.h"

template<typename T>
inline QSharedPointer<T> ptr_cast(QObject *obj=nullptr)
{
    return QSharedPointer<T>(static_cast<T *>(obj));
}

using MediaWriterPtr = QSharedPointer<MediaWriter>;

class MultiSinkElementPrivate
{
    public:
        MultiSinkElement *self;
        MultiSinkElementSettings m_settings;
        QString m_location;
        MediaWriterPtr m_mediaWriter;
        MultiSinkUtils m_utils;
        QList<int> m_inputStreams;

        // Formats and codecs info cache.
        QStringList m_supportedFormats;
        QMap<QString, QStringList> m_fileExtensions;
        QMap<QString, QString> m_formatDescription;
        QStringList m_supportedCodecs;
        QMap<QString, QString> m_codecDescription;
        QMap<QString, QString> m_codecType;
        QMap<QString, QVariantMap> m_defaultCodecParams;

        explicit MultiSinkElementPrivate(MultiSinkElement *self);
        void codecLibUpdated(const QString &codecLib);
};

MultiSinkElement::MultiSinkElement():
    AkElement()
{
    this->d = new MultiSinkElementPrivate(this);
    QObject::connect(&this->d->m_settings,
                     &MultiSinkElementSettings::codecLibChanged,
                     [this] (const QString &codecLib) {
                        this->d->codecLibUpdated(codecLib);
                     });

    this->d->codecLibUpdated(this->d->m_settings.codecLib());
}

MultiSinkElement::~MultiSinkElement()
{
    this->setState(AkElement::ElementStateNull);
    delete this->d;
}

QString MultiSinkElement::location() const
{
    return this->d->m_location;
}

QString MultiSinkElement::defaultFormat() const
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->defaultFormat();
}

QStringList MultiSinkElement::supportedFormats() const
{
    return this->d->m_supportedFormats;
}

QString MultiSinkElement::outputFormat() const
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->outputFormat();
}

QVariantList MultiSinkElement::streams()
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->streams();
}

QStringList MultiSinkElement::formatsBlackList() const
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->formatsBlackList();
}

QStringList MultiSinkElement::codecsBlackList() const
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->codecsBlackList();
}

QStringList MultiSinkElement::fileExtensions(const QString &format) const
{
    return this->d->m_fileExtensions.value(format);
}

QString MultiSinkElement::formatDescription(const QString &format) const
{
    return this->d->m_formatDescription.value(format);
}

QVariantList MultiSinkElement::formatOptions() const
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->formatOptions();
}

QStringList MultiSinkElement::supportedCodecs(const QString &format,
                                              const QString &type)
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->supportedCodecs(format, type);
}

QString MultiSinkElement::defaultCodec(const QString &format,
                                       const QString &type)
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->defaultCodec(format, type);
}

QString MultiSinkElement::codecDescription(const QString &codec) const
{
    return this->d->m_codecDescription.value(codec);
}

QString MultiSinkElement::codecType(const QString &codec) const
{
    return this->d->m_codecType.value(codec);
}

QVariantMap MultiSinkElement::defaultCodecParams(const QString &codec) const
{
    return this->d->m_defaultCodecParams.value(codec);
}

QVariantMap MultiSinkElement::addStream(int streamIndex,
                                        const AkCaps &streamCaps,
                                        const QVariantMap &codecParams)
{
    if (!this->d->m_mediaWriter)
        return {};

    auto stream =
        this->d->m_mediaWriter->addStream(streamIndex,
                                          streamCaps,
                                          codecParams);

    if (!stream.isEmpty())
        this->d->m_inputStreams << streamIndex;

    return stream;
}

QVariantMap MultiSinkElement::updateStream(int index,
                                           const QVariantMap &codecParams)
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->updateStream(index, codecParams);
}

QVariantList MultiSinkElement::codecOptions(int index)
{
    if (!this->d->m_mediaWriter)
        return {};

    return this->d->m_mediaWriter->codecOptions(index);
}

QString MultiSinkElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/MultiSink/share/qml/main.qml");
}

void MultiSinkElement::controlInterfaceConfigure(QQmlContext *context,
                                                 const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("MultiSink", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("MultiSinkUtils", const_cast<QObject *>(qobject_cast<const QObject *>(&this->d->m_utils)));
    context->setContextProperty("controlId", this->objectName());
}

void MultiSinkElement::setLocation(const QString &location)
{
    if (this->d->m_location == location)
        return;

    this->d->m_location = location;
    emit this->locationChanged(location);
}

void MultiSinkElement::setOutputFormat(const QString &outputFormat)
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->setOutputFormat(outputFormat);
}

void MultiSinkElement::setFormatOptions(const QVariantMap &formatOptions)
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->setFormatOptions(formatOptions);
}

void MultiSinkElement::setCodecOptions(int index,
                                       const QVariantMap &codecOptions)
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->setCodecOptions(index, codecOptions);
}

void MultiSinkElement::setFormatsBlackList(const QStringList &formatsBlackList)
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->setFormatsBlackList(formatsBlackList);
}

void MultiSinkElement::setCodecsBlackList(const QStringList &codecsBlackList)
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->setCodecsBlackList(codecsBlackList);
}

void MultiSinkElement::resetLocation()
{
    this->setLocation("");
}

void MultiSinkElement::resetOutputFormat()
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->resetOutputFormat();
}

void MultiSinkElement::resetFormatOptions()
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->resetFormatOptions();
}

void MultiSinkElement::resetCodecOptions(int index)
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->resetCodecOptions(index);
}

void MultiSinkElement::resetFormatsBlackList()
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->resetFormatsBlackList();
}

void MultiSinkElement::resetCodecsBlackList()
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->resetCodecsBlackList();
}

void MultiSinkElement::clearStreams()
{
    if (this->d->m_mediaWriter)
        this->d->m_mediaWriter->clearStreams();

    this->d->m_inputStreams.clear();
}

AkPacket MultiSinkElement::iStream(const AkPacket &packet)
{
    if (this->state() != ElementStatePlaying)
        return AkPacket();

    if (this->d->m_mediaWriter && this->d->m_inputStreams.contains(packet.index()))
        this->d->m_mediaWriter->enqueuePacket(packet);

    return AkPacket();
}

bool MultiSinkElement::setState(AkElement::ElementState state)
{
    if (!this->d->m_mediaWriter)
        return false;

    AkElement::ElementState curState = this->state();

    if (curState == AkElement::ElementStateNull) {
        if (state != AkElement::ElementStateNull
            && (!this->d->m_mediaWriter->init())) {
            return false;
        }
    } else {
        if (state == AkElement::ElementStateNull)
            this->d->m_mediaWriter->uninit();
    }

    return AkElement::setState(state);
}

MultiSinkElementPrivate::MultiSinkElementPrivate(MultiSinkElement *self):
    self(self)
{

}

void MultiSinkElementPrivate::codecLibUpdated(const QString &codecLib)
{
    auto state = self->state();
    self->setState(AkElement::ElementStateNull);

    QString location;

    if (this->m_mediaWriter)
        location = this->m_mediaWriter->location();

    this->m_mediaWriter =
            ptr_cast<MediaWriter>(MultiSinkElement::loadSubModule("MultiSink",
                                                                  codecLib));

    if (!this->m_mediaWriter)
        return;

    this->m_supportedFormats.clear();
    this->m_fileExtensions.clear();
    this->m_formatDescription.clear();
    this->m_supportedCodecs.clear();
    this->m_codecDescription.clear();
    this->m_codecType.clear();
    this->m_defaultCodecParams.clear();

    for (auto &format: this->m_mediaWriter->supportedFormats()) {
        this->m_supportedFormats << format;
        this->m_fileExtensions[format] =
                this->m_mediaWriter->fileExtensions(format);
        this->m_formatDescription[format] =
                this->m_mediaWriter->formatDescription(format);

        for (auto &codec: this->m_mediaWriter->supportedCodecs(format))
            if (!this->m_supportedCodecs.contains(codec)) {
                this->m_supportedCodecs << codec;
                this->m_codecDescription[codec] =
                        this->m_mediaWriter->codecDescription(codec);
                this->m_codecType[codec] =
                        this->m_mediaWriter->codecType(codec);
                this->m_defaultCodecParams[codec] =
                        this->m_mediaWriter->defaultCodecParams(codec);
            }
    }

    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::locationChanged,
                     self,
                     &MultiSinkElement::locationChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::defaultFormatChanged,
                     self,
                     &MultiSinkElement::defaultFormatChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::outputFormatChanged,
                     self,
                     &MultiSinkElement::outputFormatChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::formatOptionsChanged,
                     self,
                     &MultiSinkElement::formatOptionsChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::codecOptionsChanged,
                     self,
                     &MultiSinkElement::codecOptionsChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::streamsChanged,
                     self,
                     &MultiSinkElement::streamsChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::formatsBlackListChanged,
                     self,
                     &MultiSinkElement::formatsBlackListChanged);
    QObject::connect(this->m_mediaWriter.data(),
                     &MediaWriter::codecsBlackListChanged,
                     self,
                     &MultiSinkElement::formatsBlackListChanged);
    QObject::connect(self,
                     &MultiSinkElement::locationChanged,
                     this->m_mediaWriter.data(),
                     &MediaWriter::setLocation);
    QObject::connect(self,
                     &MultiSinkElement::formatOptionsChanged,
                     this->m_mediaWriter.data(),
                     &MediaWriter::setFormatOptions);

    this->m_mediaWriter->setLocation(location);
    emit self->supportedFormatsChanged(self->supportedFormats());

    self->setState(state);
}

#include "moc_multisinkelement.cpp"
