/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.com>
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

#ifndef TELEPATHY_ACCOUNTS_KCM_ACCOUNTS_LIST_MODEL_H
#define TELEPATHY_ACCOUNTS_KCM_ACCOUNTS_LIST_MODEL_H

#include <QtCore/QAbstractListModel>

#include <TelepathyQt/Account>

#include <KTp/ktp-export.h>

class KIcon;

class KTP_EXPORT AccountsListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsListModel);

public:
    enum Roles {
        ConnectionStateRole = Qt::UserRole,
        ConnectionStateDisplayRole = Qt::UserRole+1,
        ConnectionStateIconRole,
        ConnectionErrorMessageDisplayRole,
        ConnectionProtocolNameRole,
        AccountRole
    };

    explicit AccountsListModel(QObject *parent = 0);
    virtual ~AccountsListModel();

    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private Q_SLOTS:
    void onAccountAdded(const Tp::AccountPtr &account);
    void onAccountItemRemoved();
    void onAccountItemUpdated();

private:
    class Private;
    Private * const d;

    const QString connectionStateString(const Tp::AccountPtr &account) const;
    const KIcon connectionStateIcon(const Tp::AccountPtr &account) const;
    const QString connectionStatusReason(const Tp::AccountPtr &account) const;
};

Q_DECLARE_METATYPE(Tp::AccountPtr)

#endif // header guard

