/*
 * Copyright (C) 2012 Dan Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "actions.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/ChannelRequestHints>
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingFailure>
#include <TelepathyQt/ReferencedHandles>

#include <QMimeType>
#include <KToolInvocation>
#include "ktp-debug.h"
#include <KLocalizedString>
#include <KNotification>
#include <KAboutData>

#define PREFERRED_TEXT_CHAT_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.KTp.TextUi")
#define PREFERRED_FILE_TRANSFER_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.KTp.FileTransfer")
#define PREFERRED_AUDIO_VIDEO_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.KTp.CallUi")
#define PREFERRED_RFB_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.krfb_rfb_handler")

using namespace KTp;

Tp::PendingChannelRequest* Actions::startChat(const Tp::AccountPtr &account,
                                              const QString &contactIdentifier,
                                              bool delegateToPreferredHandler)
{
    if (account.isNull() || contactIdentifier.isEmpty()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting text channel for contact id: " << contactIdentifier;

    Tp::ChannelRequestHints hints;
    if (delegateToPreferredHandler) {
      hints.setHint(QLatin1String("org.freedesktop.Telepathy.ChannelRequest"),
                    QLatin1String("DelegateToPreferredHandler"),
                    QVariant(true));
    }

    return account->ensureTextChat(contactIdentifier,
                                   QDateTime::currentDateTime(),
                                   PREFERRED_TEXT_CHAT_HANDLER,
                                   hints);
}

Tp::PendingChannelRequest* Actions::startChat(const Tp::AccountPtr &account,
                                              const Tp::ContactPtr &contact,
                                              bool delegateToPreferredHandler)
{
    if (account.isNull() || contact.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting text channel for" << contact->id();

    Tp::ChannelRequestHints hints;
    if (delegateToPreferredHandler) {
      hints.setHint(QLatin1String("org.freedesktop.Telepathy.ChannelRequest"),
                    QLatin1String("DelegateToPreferredHandler"),
                    QVariant(true));
    }

    return account->ensureTextChat(contact,
                                   QDateTime::currentDateTime(),
                                   PREFERRED_TEXT_CHAT_HANDLER,
                                   hints);
}

Tp::PendingChannelRequest* Actions::startGroupChat(const Tp::AccountPtr &account,
                                                   const QString &roomName)
{
    if (account.isNull() || roomName.isEmpty()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting text chat room " << roomName;

    Tp::ChannelRequestHints hints;
    hints.setHint(QLatin1String("org.kde.telepathy"), QLatin1String("forceRaiseWindow"), QVariant(true));

    return account->ensureTextChatroom(roomName,
                                       QDateTime::currentDateTime(),
                                       PREFERRED_TEXT_CHAT_HANDLER,
                                       hints);
}

Tp::PendingChannelRequest* Actions::startAudioCall(const Tp::AccountPtr &account,
                                                   const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting audio channel for" << contact->id();

    return account->ensureAudioCall(contact,
                                    QLatin1String("audio"),
                                    QDateTime::currentDateTime(),
                                    PREFERRED_AUDIO_VIDEO_HANDLER);
}

Tp::PendingChannelRequest* Actions::startAudioVideoCall(const Tp::AccountPtr &account,
                                                        const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting audio-video channel for" << contact->id();

    return account->ensureAudioVideoCall(contact,
                                         QLatin1String("audio"),
                                         QLatin1String("video"),
                                         QDateTime::currentDateTime(),
                                         PREFERRED_AUDIO_VIDEO_HANDLER);
}

Tp::PendingChannelRequest* Actions::startDesktopSharing(const Tp::AccountPtr &account,
                                                        const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting stream tube for" << contact->id();

    return account->createStreamTube(contact,
                                     QLatin1String("rfb"),
                                     QDateTime::currentDateTime(),
                                     PREFERRED_RFB_HANDLER);
}

Tp::PendingChannelRequest* Actions::startFileTransfer(const Tp::AccountPtr &account,
                                                      const Tp::ContactPtr &contact,
                                                      const QString &filePath)
{
    if (account.isNull() || contact.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting file transfer of" << filePath << "to" << contact->id();

    QFileInfo fileInfo(filePath);

    Tp::FileTransferChannelCreationProperties fileTransferProperties;

    if (account->serviceName() == QLatin1String("google-talk") &&
        (fileInfo.suffix() == QLatin1String("exe") || fileInfo.suffix() == QLatin1String("ini"))) {

        qCDebug(KTP_COMMONINTERNALS) << "Google Talk forbids transfering files with suffix \"ini\" or \"exe\". Renaming.";

        QString fileName = fileInfo.fileName().append(QLatin1String("_"));

        QMimeDatabase db;
        fileTransferProperties = Tp::FileTransferChannelCreationProperties(fileName,
                                                                           db.mimeTypeForFile(filePath).name(),
                                                                           fileInfo.size());

        fileTransferProperties.setUri(QUrl::fromLocalFile(filePath).toString());
        fileTransferProperties.setLastModificationTime(fileInfo.lastModified());

        KNotification *notification = new KNotification (QLatin1String("googletalkExtensionsError"));
        notification->setText(i18n("Transferring files with .exe or .ini extension is not allowed by Google Talk. It was sent with filename <i>%1</i>", fileName));
        notification->setTitle(i18n("Transferred file renamed"));

        notification->setComponentName(QStringLiteral("ktelepathy"));
        notification->sendEvent();

    } else {
        QMimeDatabase db;
        fileTransferProperties = Tp::FileTransferChannelCreationProperties(
                                    filePath, db.mimeTypeForFile(filePath, QMimeDatabase::MatchContent).name());
    }

    return account->createFileTransfer(contact,
                                       fileTransferProperties,
                                       QDateTime::currentDateTime(),
                                       PREFERRED_FILE_TRANSFER_HANDLER);
}

Tp::PendingOperation* Actions::startFileTransfer(const Tp::AccountPtr &account,
                                                 const Tp::ContactPtr &contact,
                                                 const QUrl &url)
{
    if (account.isNull() || contact.isNull() || url.isEmpty()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
    }

    qCDebug(KTP_COMMONINTERNALS) << "Requesting file transfer of" << url.toLocalFile() << "to" << contact->id();

    Tp::PendingOperation *ret = 0;
    if (url.isLocalFile()) {
        ret = startFileTransfer(account, contact, url.toLocalFile());
    } else {
        ret = new Tp::PendingFailure(QLatin1String("Failed file transfer"), QString(QLatin1String("You are only supposed to send local files, not %1")).arg(url.toString()), account);
    }
    return ret;
}

void Actions::openLogViewer(const Tp::AccountPtr &account,
                            const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Parameters invalid";
        return;
    }

    openLogViewer(account, contact->id());
}

void Actions::openLogViewer(const QUrl &uri)
{
    qCDebug(KTP_COMMONINTERNALS) << "Opening logviewer for" << uri;

    QStringList arguments;
    arguments << QLatin1String("--") << uri.toString();

    KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"), arguments);
}

void Actions::openLogViewer(const Tp::AccountPtr& account, const QString& targetId)
{
    if (account.isNull() || targetId.isEmpty()) {
        qCWarning(KTP_COMMONINTERNALS) << " Parameters invalid";
        return;
    }

    qCDebug(KTP_COMMONINTERNALS) << "Opening logviewer for" << targetId;

    QStringList arguments;
    arguments << QLatin1String("--") << account->uniqueIdentifier() << targetId;

    /* Use "--" so that KCmdLineArgs does not parse UIDs starting with "-" as arguments */
    KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"), arguments);
}


const QVariantMap createHintsForCollabRequest(const Actions::DocumentList& documents, bool needOpenEditor)
{
    QVariantMap hints;
    hints.insert(QLatin1String("initialDocumentsSize"), documents.size());
    for ( int i = 0; i < documents.size(); i++ ) {
        const QString key(QLatin1String("initialDocument") + QString::number(i));
        hints.insert(key, documents.at(i).fileName());
        if ( needOpenEditor ) {
            hints.insert(key + QLatin1String("_source"), documents.at(i).url());
        }
    }
    if ( needOpenEditor ) {
        hints.insert(QLatin1String("needToOpenDocument"), QVariant(true));
    }
    return hints;
}

Tp::PendingChannelRequest* createCollabRequest(const Tp::AccountPtr account,
                                               const Actions::DocumentList documents,
                                               QVariantMap requestBase,
                                               bool needOpenEditor)
{
    QVariantMap hints = createHintsForCollabRequest(documents, needOpenEditor);

    requestBase.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"),
                    TP_QT_IFACE_CHANNEL_TYPE_STREAM_TUBE);
    requestBase.insert(TP_QT_IFACE_CHANNEL_TYPE_STREAM_TUBE + QLatin1String(".Service"),
                    QLatin1String("infinote"));

    Tp::PendingChannelRequest* channelRequest;
    channelRequest = account->ensureChannel(requestBase,
                                            QDateTime::currentDateTime(),
                                            QLatin1String("org.freedesktop.Telepathy.Client.KTp.infinoteServer"),
                                            hints);

    return channelRequest;
}

Tp::PendingChannelRequest* Actions::startCollaborativeEditing(const Tp::AccountPtr& account,
                                                              const Tp::ContactPtr& contact,
                                                              const DocumentList& documents,
                                                              bool needOpenEditor)
{
    QVariantMap request;
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                (uint) Tp::HandleTypeContact);
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandle"),
                contact->handle().at(0));
    return createCollabRequest(account, documents, request, needOpenEditor);
}

Tp::PendingChannelRequest* Actions::startCollaborativeEditing(const Tp::AccountPtr& account,
                                                              const QString& chatroom,
                                                              const DocumentList& documents,
                                                              bool needOpenEditor)
{
    QVariantMap request;
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                (uint) Tp::HandleTypeRoom);
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetID"),
                chatroom);
    return createCollabRequest(account, documents, request, needOpenEditor);
}
