/*
 * This file is part of telepathy-integration-daemon
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#include "telepathyaccount.h"

#include "pimo.h"

#include <Nepomuk/Thing>

TelepathyAccount::TelepathyAccount(const QString &path, QObject *parent)
 : QObject(parent),
   m_path(path)
{
    // FIXME: Check there is an akonadi resource for this account, and create one if not.

    // Check that this Account is set up in nepomuk.
    doNepomukSetup();
}

TelepathyAccount::~TelepathyAccount()
{
}

void TelepathyAccount::doNepomukSetup()
{
    // Get the PIMO:Person for "me" from nepomuk
    Nepomuk::Thing me(QUrl::fromEncoded("nepomuk:/myself"));

    if (!me.exists()) {
        // The PIMO:Person representing "me" does not exist, so we need to create it.
        me.addType(Nepomuk::Vocabulary::PIMO::Person());
    }
}

