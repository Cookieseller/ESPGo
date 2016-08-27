#include "stdafx.h"
#include "NetMessageDecoder.h"

CNetMessageDecoder::CNetMessageDecoder()
{
	m_nServerClassBits = 0;
}

EntityEntry* CNetMessageDecoder::FindEntity(int nEntity)
{
	for (std::vector<EntityEntry *>::iterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		if ((*i)->m_nEntity == nEntity)
			return *i;
	}

	return NULL;
}

int CNetMessageDecoder::ReadFieldIndex(CBitRead &entityBitBuffer, int lastIndex, bool bNewWay)
{
	int ret = 0;

	if (bNewWay && entityBitBuffer.ReadOneBit())
	{
		return lastIndex + 1;
	}
	else
	{
		ret = entityBitBuffer.ReadUBitLong(7); // read 7 bits
		switch (ret & (32 | 64))
		{
		case 32:
			ret = (ret &~96) | (entityBitBuffer.ReadUBitLong(2) << 5);
			assert(ret >= 32);
			break;
		case 64:
			ret = (ret &~96) | (entityBitBuffer.ReadUBitLong(4) << 5);
			assert(ret >= 128);
			break;
		case 96:
			ret = (ret &~96) | (entityBitBuffer.ReadUBitLong(7) << 5);
			assert(ret >= 512);
			break;
		}
	}

	if (ret == 0xFFF) // end marker is 4095 for cs:go
		return -1;

	return lastIndex + 1 + ret;
}

CSVCMsg_SendTable* CNetMessageDecoder::GetTableByClassID(uint32 nClassID)
{
	for (uint32 i = 0; i < m_serverClasses.size(); i++)
	{
		if (m_serverClasses[i].nClassID == nClassID)
			return &(m_dataTables[m_serverClasses[i].nDataTable]);
	}

	return NULL;
}

FlattenedPropEntry* CNetMessageDecoder::GetSendPropByIndex(uint32 uClass, uint32 uIndex)
{
	if (uIndex < m_serverClasses[uClass].flattenedProps.size())
		return &m_serverClasses[uClass].flattenedProps[uIndex];
	
	return NULL;
}

bool CNetMessageDecoder::ReadNewEntity(CBitRead &entityBitBuffer, EntityEntry *pEntity)
{
	bool bNewWay = (entityBitBuffer.ReadOneBit() == 1);  // 0 = old way, 1 = new way

	std::vector<int> fieldIndices;

	int index = -1;
	do
	{
		index = ReadFieldIndex(entityBitBuffer, index, bNewWay);
		if (index != -1)
		{
			fieldIndices.push_back(index);
		}
	} while (index != -1);

	CSVCMsg_SendTable *pTable = GetTableByClassID(pEntity->m_uClass);
	for (unsigned int i = 0; i < fieldIndices.size(); i++)
	{
		FlattenedPropEntry *pSendProp = GetSendPropByIndex(pEntity->m_uClass, fieldIndices[i]);
		if (pSendProp)
		{
			Prop_t *pProp = DecodeProp(entityBitBuffer, pSendProp, pEntity->m_uClass, fieldIndices[i], !g_bDumpPacketEntities);
			pEntity->AddOrUpdateProp(pSendProp, pProp);
		}
		else
		{
			return false;
		}
	}

	return true;
}

EntityEntry* CNetMessageDecoder::AddEntity(int nEntity, uint32 uClass, uint32 uSerialNum)
{
	// if entity already exists, then replace it, else add it
	EntityEntry *pEntity = FindEntity(nEntity);
	if (pEntity)
	{
		pEntity->m_uClass = uClass;
		pEntity->m_uSerialNum = uSerialNum;
	}
	else
	{
		pEntity = new EntityEntry(nEntity, uClass, uSerialNum);
		m_entities.push_back(pEntity);
	}

	return pEntity;
}

void CNetMessageDecoder::RemoveEntity(int nEntity)
{
	for (std::vector< EntityEntry * >::iterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		EntityEntry *pEntity = *i;
		if (pEntity->m_nEntity == nEntity)
		{
			m_entities.erase(i);
			delete pEntity;
			break;
		}
	}
}

bool CNetMessageDecoder::ReadFromBuffer(CBitRead &buffer, void **pBuffer, int& size)
{
	size = buffer.ReadVarInt32();
	if (size < 0 || size > NET_MAX_PAYLOAD)
		return false;

	// Check its valid
	if (size > buffer.GetNumBytesLeft())
		return false;

	*pBuffer = malloc(size);

	// If the read buffer is byte aligned, we can parse right out of it
	if ((buffer.GetNumBitsRead() % 8) == 0)
	{
		memcpy(*pBuffer, buffer.GetBasePointer() + buffer.GetNumBytesRead(), size);
		buffer.SeekRelative(size * 8);
		return true;
	}

	// otherwise we have to ReadBytes() it out
	if (!buffer.ReadBytes(*pBuffer, size))
	{
		return false;
	}

	return true;
}

CSVCMsg_SendTable* CNetMessageDecoder::GetTableByName(const char *pName)
{
	for (unsigned int i = 0; i < m_dataTables.size(); i++)
	{
		if (m_dataTables[i].net_table_name().compare(pName) == 0)
			return &(m_dataTables[i]);
	}

	return NULL;
}

void CNetMessageDecoder::GatherExcludes(CSVCMsg_SendTable *pTable)
{
	for (int iProp = 0; iProp < pTable->props_size(); iProp++)
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props(iProp);
		if (sendProp.flags() & SPROP_EXCLUDE)
			m_currentExcludes.push_back(ExcludeEntry(sendProp.var_name().c_str(), sendProp.dt_name().c_str(), pTable->net_table_name().c_str()));

		if (sendProp.type() == DPT_DataTable)
		{
			CSVCMsg_SendTable *pSubTable = GetTableByName(sendProp.dt_name().c_str());
			if (pSubTable != NULL)
				GatherExcludes(pSubTable);
		}
	}
}

void CNetMessageDecoder::GatherProps(CSVCMsg_SendTable *pTable, int nServerClass)
{
	std::vector< FlattenedPropEntry > tempFlattenedProps;
	GatherProps_IterateProps(pTable, nServerClass, tempFlattenedProps);

	std::vector< FlattenedPropEntry > &flattenedProps = m_serverClasses[nServerClass].flattenedProps;
	for (uint32 i = 0; i < tempFlattenedProps.size(); i++)
	{
		flattenedProps.push_back(tempFlattenedProps[i]);
	}
}

bool CNetMessageDecoder::IsPropExcluded(CSVCMsg_SendTable *pTable, const CSVCMsg_SendTable::sendprop_t &checkSendProp)
{
	for (unsigned int i = 0; i < m_currentExcludes.size(); i++)
	{
		if (pTable->net_table_name().compare(m_currentExcludes[i].m_pDTName) == 0 &&
			checkSendProp.var_name().compare(m_currentExcludes[i].m_pVarName) == 0)
			return true;
	}
	return false;
}


void CNetMessageDecoder::GatherProps_IterateProps(CSVCMsg_SendTable *pTable, int nServerClass, std::vector< FlattenedPropEntry > &flattenedProps)
{
	for (int iProp = 0; iProp < pTable->props_size(); iProp++)
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props(iProp);

		if ((sendProp.flags() & SPROP_INSIDEARRAY) ||
			(sendProp.flags() & SPROP_EXCLUDE) ||
			IsPropExcluded(pTable, sendProp))
			continue;

		if (sendProp.type() == DPT_DataTable)
		{
			CSVCMsg_SendTable *pSubTable = GetTableByName(sendProp.dt_name().c_str());
			if (pSubTable != NULL)
			{
				if (sendProp.flags() & SPROP_COLLAPSIBLE)
					GatherProps_IterateProps(pSubTable, nServerClass, flattenedProps);
				else
					GatherProps(pSubTable, nServerClass);
			}
		}
		else
		{
			if (sendProp.type() == DPT_Array)
				flattenedProps.push_back(FlattenedPropEntry(&sendProp, &(pTable->props(iProp - 1))));
			else
				flattenedProps.push_back(FlattenedPropEntry(&sendProp, NULL));
		}
	}
}

void CNetMessageDecoder::FlattenDataTable(int nServerClass)
{
	CSVCMsg_SendTable *pTable = &m_dataTables[m_serverClasses[nServerClass].nDataTable];

	m_currentExcludes.clear();
	GatherExcludes(pTable);

	GatherProps(pTable, nServerClass);

	std::vector< FlattenedPropEntry > &flattenedProps = m_serverClasses[nServerClass].flattenedProps;

	// get priorities
	std::vector< uint32 > priorities;
	priorities.push_back(64);
	for (unsigned int i = 0; i < flattenedProps.size(); i++)
	{
		uint32 priority = flattenedProps[i].m_prop->priority();

		bool bFound = false;
		for (uint32 j = 0; j < priorities.size(); j++)
		{
			if (priorities[j] == priority)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
			priorities.push_back(priority);
	}

	std::sort(priorities.begin(), priorities.end());

	// sort flattenedProps by priority
	uint32 start = 0;
	for (uint32 priority_index = 0; priority_index < priorities.size(); ++priority_index)
	{
		uint32 priority = priorities[priority_index];

		while (true)
		{
			uint32 currentProp = start;
			while (currentProp < flattenedProps.size())
			{
				const CSVCMsg_SendTable::sendprop_t *prop = flattenedProps[currentProp].m_prop;

				if (prop->priority() == priority || (priority == 64 && (SPROP_CHANGES_OFTEN & prop->flags())))
				{
					if (start != currentProp)
					{
						FlattenedPropEntry temp = flattenedProps[start];
						flattenedProps[start] = flattenedProps[currentProp];
						flattenedProps[currentProp] = temp;
					}
					start++;
					break;
				}
				currentProp++;
			}

			if (currentProp == flattenedProps.size())
				break;
		}
	}
}

bool CNetMessageDecoder::ParseDataTable(CBitRead &buffer)
{
	CSVCMsg_SendTable msg;
	while (1)
	{
		int type = buffer.ReadVarInt32();

		void *pBuffer = NULL;
		int size = 0;
		if (!ReadFromBuffer(buffer, &pBuffer, size))
		{
			printf("ParseDataTable: ReadFromBuffer failed.\n");
			return false;
		}
		msg.ParseFromArray(pBuffer, size);
		free(pBuffer);

		if (msg.is_end())
			break;

		m_dataTables.push_back(msg);
	}

	short nServerClasses = buffer.ReadShort();
	assert(nServerClasses);
	for (int i = 0; i < nServerClasses; i++)
	{
		ServerClass_t entry;
		entry.nClassID = buffer.ReadShort();
		if (entry.nClassID >= nServerClasses)
		{
			printf("ParseDataTable: invalid class index (%d).\n", entry.nClassID);
			return false;
		}

		int nChars;
		buffer.ReadString(entry.strName, sizeof(entry.strName), false, &nChars);
		buffer.ReadString(entry.strDTName, sizeof(entry.strDTName), false, &nChars);

		// find the data table by name
		entry.nDataTable = -1;
		for (unsigned int j = 0; j < m_dataTables.size(); j++)
		{
			if (strcmp(entry.strDTName, m_dataTables[j].net_table_name().c_str()) == 0)
			{
				entry.nDataTable = j;
				break;
			}
		}

		m_serverClasses.push_back(entry);
	}


	for (int i = 0; i < nServerClasses; i++)
	{
		FlattenDataTable(i);
	}

	// perform integer log2() to set m_nServerClassBits
	int nTemp = nServerClasses;
	m_nServerClassBits = 0;
	while (nTemp >>= 1) ++m_nServerClassBits;

	m_nServerClassBits++;

	return true;
}

void CNetMessageDecoder::DecodeNetMessage(const void *parseBuffer, int bufferSize, std::list<EntityEntry> &entities)
{
	CSVCMsg_PacketEntities msg;

	if (msg.ParseFromArray(parseBuffer, bufferSize))
	{
		CBitRead entityBitBuffer(&msg.entity_data()[0], msg.entity_data().size());
		bool isDelta = msg.is_delta();
		int nHeaderCount = msg.updated_entries();
		int nHeaderBase = -1;
		int newEntity = -1;
		int UpdateFlags = 0;

		UpdateType updateType = PreserveEnt;

		while (updateType < Finished)
		{
			nHeaderCount--;

			bool bIsEntity = (nHeaderCount >= 0) ? true : false;

			if (bIsEntity)
			{
				UpdateFlags = FHDR_ZERO;

				newEntity = nHeaderBase + 1 + entityBitBuffer.ReadUBitVar();
				nHeaderBase = newEntity;

				// leave pvs flag
				if (entityBitBuffer.ReadOneBit() == 0)
				{
					// enter pvs flag
					if (entityBitBuffer.ReadOneBit() != 0)
						UpdateFlags |= FHDR_ENTERPVS;
				}
				else
				{
					UpdateFlags |= FHDR_LEAVEPVS;

					// Force delete flag
					if (entityBitBuffer.ReadOneBit() != 0)
						UpdateFlags |= FHDR_DELETE;
				}
			}

			for (updateType = PreserveEnt; updateType == PreserveEnt; )
			{
				// Figure out what kind of an update this is.
				if (!bIsEntity || newEntity > ENTITY_SENTINEL)
				{
					updateType = Finished;
				}
				else
				{
					if (UpdateFlags & FHDR_ENTERPVS)
					{
						if (!EntityEnterPVS(entityBitBuffer, newEntity))
						{
							fprintf(stderr, "Error reading entity! Bailing on this PacketEntities!");
							return;
						}
					}
					else if (UpdateFlags & FHDR_LEAVEPVS)
					{
						if (!EntityLeavePVS(newEntity, isDelta))
							updateType = Failed;
					}
					else
					{
						EntityDelta(entityBitBuffer, newEntity);
						fprintf(stderr, "Error reading entity! Bailing on this PacketEntities!");
					}

				}
			}
		}
	}
}

bool CNetMessageDecoder::EntityEnterPVS(CBitRead entityBitBuffer, int newEntity)
{
	uint32 uClass = entityBitBuffer.ReadUBitLong(m_nServerClassBits);
	uint32 uSerialNum = entityBitBuffer.ReadUBitLong(NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS);

	EntityEntry *pEntity = AddEntity(newEntity, uClass, uSerialNum);

	return ReadNewEntity(entityBitBuffer, pEntity);
}

bool CNetMessageDecoder::EntityLeavePVS(int newEntity, bool isDelta)
{
	if (!isDelta)  // Should never happen on a full update.
	{
		assert(!"WARNING: LeavePVS on full update");
		return false;
	}
	RemoveEntity(newEntity);

	return true;
}

bool CNetMessageDecoder::EntityDelta(CBitRead entityBitBuffer, int newEntity)
{
	EntityEntry *pEntity = FindEntity(newEntity);
	if (pEntity)
	{
		if (!ReadNewEntity(entityBitBuffer, pEntity))
		{
			return false;
		}
		printf("Entity Delta update: id:%d, class:%d, serial:%d\n", pEntity->m_nEntity, pEntity->m_uClass, pEntity->m_uSerialNum);
		//entities.push_back(*pEntity);
	}
	else
	{
		assert(0);
	}
}

