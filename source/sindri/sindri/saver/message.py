# encoding: utf-8

# Copyright (c) 2001-2014, Canal TP and/or its affiliates. All rights reserved.
#
# This file is part of Navitia,
#     the software to build cool stuff with public transport.
#
# Hope you'll enjoy and contribute to this project,
#     powered by Canal TP (www.canaltp.fr).
# Help us simplify mobility and open public transport:
#     a non ending quest to the responsive locomotion way of traveling!
#
# LICENCE: This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Stay tuned using
# twitter @navitia
# IRC #navitia on freenode
# https://groups.google.com/d/forum/navitia
# www.navitia.io

from sqlalchemy import select
from sindri.saver.utils import parse_active_days, from_timestamp, from_time, \
    FunctionalError


def build_message_dict(message):
    """
    construit à partir d'un object protobuf pbnavitia.realtime.Message le
    dictionnaire utilisé pour l'insertion en base
    """
    result = {}
    result['uri'] = message.uri
    result['object_uri'] = message.object.object_uri
    result['object_type_id'] = message.object.object_type

    result['active_days'] = parse_active_days(message.active_days)

    result['start_publication_date'] = from_timestamp(
        message.start_publication_date)
    result['end_publication_date'] = from_timestamp(
        message.end_publication_date)

    result['start_application_date'] = from_timestamp(
        message.start_application_date)
    result['end_application_date'] = from_timestamp(
        message.end_application_date)

    result['start_application_daily_hour'] = from_time(
        message.start_application_daily_hour)
    result['end_application_daily_hour'] = from_time(
        message.end_application_daily_hour)

    result['message_status_id'] = message.message_status

    return result


def build_localized_message_dict(localized_msg, message_id):
    """
    construit à partir d'un object protobuf pbnavitia.realtime.LocalizeMessage
    le dictionnaire utilisé pour l'insertion en base
    """
    result = {}
    result['message_id'] = message_id
    result['language'] = localized_msg.language
    result['body'] = localized_msg.body if localized_msg.body else None
    result['title'] = localized_msg.title if localized_msg.title else None
    return result


def persist_message(meta, conn, message):
    """
    enregistre en base le message
    """
    row_id = find_message_id(meta, conn, message.uri)
    save_message(meta, conn, row_id, message)


def find_message_id(meta, conn, message_uri):
    """
    retourne l'id en base du message correspondant à cette URI
    si celui ci est présent dans ED sinon retourne None
    """
    msg_table = meta.tables['realtime.message']
    query = select([msg_table.c.id],
                   msg_table.c.uri == message_uri)
    result = conn.execute(query)
    row = result.fetchone()
    return row[0] if row else None


def save_message(meta, conn, message_id, message):
    """
    insert ou update le message en base de données
    et insert les localized_messages associés
    """
    msg_table = meta.tables['realtime.message']
    local_msg_table = meta.tables['realtime.localized_message']
    if not message_id:
        query = msg_table.insert()
    else:
        query = msg_table.update().where(msg_table.c.id == message_id)

    result = conn.execute(query.values(build_message_dict(message)))
    if result.is_insert:
        message_id = result.inserted_primary_key[0]
    else:
# si c'est un update on supprime les messages text associés avant de
# les recréer
        conn.execute(local_msg_table.delete().where(
            local_msg_table.c.message_id == message_id))

    # on insére les messages text
    query = local_msg_table.insert()
    for localized_message in message.localized_messages:
        conn.execute(query.values(
            build_localized_message_dict(localized_message, message_id)))
