#pragma once
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <Windows.h>
#include <vector>
#include <winsock.h>
#include <atlstr.h>
#include <cwchar>


using namespace std;

#define SECTORSIZE 512
#define CLUSTERSIZE 4096
#define PARTITIONTABLESIZE 16
#define PARTITIONTABLEOFFSET 446
#define PARTITION_ENTRY_SIZE 128

#define NTFS 0x07
#define GPT 0xEE
#define NO_FS 0x00
#define LINE_SEP "------------------------------------------"

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

U64 betole64(U64 num);
U32 betole32(U32 num);
U16 betole16(U16 num);

typedef struct __GPTHeader {
	U8 Signature[8];
	U8 Revision[4];
	U32 HeaderSize;
	U32 HEaderChksum;
	U32 Reserved;
	U64 LBAofGPTHeader;
	U64 LBAofbkpGPTHeader;
	U64 StartingLBAforPartitions;
	U64 EndingLBAforPratitions;
	U8 GUID[16];
	U64 StartingLBAParitionTable;
	U32 NumOfPartitionEntries;
	U32 SizeOfEntries;
	U32 PartitionTableChksum;
	U8 Padding[420];
} GPTHeader;

typedef struct __GPTable {
	U8 PartitonTypeGUID[16];
	U8 PartitionGUID[16];
	U64 PartitionStartingLBA;
	U64 PartitionEndeingLBA;
	U64 PartitionAtts;
	U8 PartitonName[72];
} GPTable;

typedef struct __BootRecord {
	U8 JumpBootCode[3];
	U8 OEM_ID[8];
	U16 BytesPerSector;
	U8 SecPerClus;
	U16 ReservedSectors;
	U8 Unused1[5];
	U8 Media;
	U8 Unused2[18];
	U64 TotalSectors;
	U64 StartClusterforMFT;
	U64 StartClusterforMFTMirr;
	U32 ClusPerEntry;
	U32 ClusPerIndex;
	U64 VolumeSerialNumber;
	U8 Unused[430];
	U16 Siganture;
	U64 BytesPerClus;
	U64 VBR_LBA;
} VBR;

typedef struct __FixupArr {
	U16 arrEntries[4];
} FixupArr;

typedef struct __MFTHeader {
	U8 Signature[4];
	U16 FixupArrOffset;
	U16 FixupArrEntries;
	U64 LSN;
	U16 SeqNum;
	U16 HardlinkCnt;
	U16 FileAttrOffset;
	U16 Flags;
	U32 RealSizeofMFTEntry;
	U32 AllocatedSizeofMFTEntry;
	U64 FileReference;
	U16 NextAttrID;
	U8 Unused[6];
	FixupArr fixupArr;
} MFTHeader;


typedef struct __attrCommonHeader {
	U32 AttrtypeID;
	U32 lenOfAttr;
	U8 Nregflag;
	U8 nameLen;
	U16 OffsettoName;
	U16 Flags;
	U16 attrID;
} attrCommonHeader;

typedef struct __residentAttrHdr {
	U32 sizeOfContent;
	U16 offsetToContent;
	U8 idxedFlag;
	U8 Padding;
} residentAttrHdr;

typedef struct __nonResidentAttr {
	U64 startVcn;
	U64 endVcn;
	U16 runListOffset;
	U16 compUnitSize;
	U32 padding;
	U64 allocatedSize;
	U64 realSize;
	U64 initSize;
} nonResidentAttrHdr;

typedef struct __STDINFO {
	U64 createTime;
	U64 modifiedTime;
	U64 mftModifiedTime;
	U64 lastAccessedTime;
	U32 Flags;
	U32 MaxVersionNum;
	U32 VersionNum;
	U32 ClassID;
	U32 OwnerID;
	U32 SecID;
	U64 QuotaCharged;
	U64 UCN;
} stdInfo;

typedef struct __FILENAME {
	U64 fileReferenceAddrofParent;
	U64 createTime;
	U64 modifiedTime;
	U64 mftModifiedTime;
	U64 lastAccessedTime;
	U64 allocatedSize;
	U64 realsSize;
	U32 flags;
	U32 reparseValue;
	U8 lenName;
	U8 nameSpace;
} fileName;
#pragma pack()

class MFTEntry {

public:
	MFTEntry(void) = default;
	MFTEntry(U32 targetNum) :mftNum(targetNum) {}

	const U32 getEntryNum(void) const { return mftNum; }

	void setMFTEntry(void * buf, uint32_t MFTSize) {
		memset(this->Buf, 0, MFTSize);
		memcpy_s(this->Buf, MFTSize, buf, MFTSize);
	}

	U8 * getBuf(void) { return Buf; }


	void printMftInfo() {
		cout << "<<<<<<<<<<<<<<<<<<<<<<<<  MFT Entry Header >>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		cout << "LSN = " << mftHdr.LSN << endl;
		cout << "Sequence Number = " << mftHdr.SeqNum << endl;
		cout << "Link Count = " << mftHdr.HardlinkCnt << endl;
		cout << "First Attr Offset = " << mftHdr.FileAttrOffset << endl;
		cout << "Flags = " << mftHdr.Flags << endl;
		cout << "Used Sizeof MFT = " << mftHdr.RealSizeofMFTEntry << endl;
		cout << "Allocated size of MFT = " << mftHdr.AllocatedSizeofMFTEntry << endl;
		cout << "File Reference to base record = " << mftHdr.FileReference << endl;
		cout << "Next Attr ID = " << mftHdr.NextAttrID << endl;
		cout << endl << "<<<<<<<<<<<<<<<<<<<<<<<<  Fixup Array >>>>>>>>>>>>>>>>>>>>>>>>" << endl;

		for (int a = 0; a < 4; a = a + 1) cout << "0x" << hex << setw(4) << setfill('0') << betole16(mftHdr.fixupArr.arrEntries[a]) << ' ';
		cout << endl << endl;
	}

	void printStdInfo() {
		this->curPtr = mftHdr.FixupArrOffset;
		this->curPtr += 8;

		attrCommonHeader cmnHdr;
		memcpy_s(static_cast<void*>(&cmnHdr), sizeof(attrCommonHeader), Buf + curPtr, sizeof(attrCommonHeader));
		this->curPtr += sizeof(attrCommonHeader);

		residentAttrHdr resHdr;
		memcpy_s(static_cast<void*>(&resHdr), sizeof(residentAttrHdr), Buf + curPtr, sizeof(residentAttrHdr));
		this->curPtr += sizeof(residentAttrHdr);

		stdInfo STDINFO;
		memcpy_s(static_cast<void*>(&STDINFO), resHdr.sizeOfContent, Buf + curPtr, resHdr.sizeOfContent);
		this->curPtr += resHdr.sizeOfContent;

		cout << "<<<<<<<<<<<<<<<<<<<<<<<<  STANDARD INFO >>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		cout << "Created Time : " << STDINFO.createTime << endl;
		cout << "Modified Time : " << STDINFO.modifiedTime << endl;
		cout << "MFT Modified Time : " << STDINFO.mftModifiedTime << endl;
		cout << "Accessed Time : " << STDINFO.lastAccessedTime << endl;

	}

	void printFileNameInfo() {
		attrCommonHeader cmnHdr;
		memcpy_s(static_cast<void*>(&cmnHdr), sizeof(attrCommonHeader), Buf + curPtr, sizeof(attrCommonHeader));
		this->curPtr += sizeof(attrCommonHeader);

		residentAttrHdr resHdr;
		memcpy_s(static_cast<void*>(&resHdr), sizeof(residentAttrHdr), Buf + curPtr, sizeof(residentAttrHdr));
		this->curPtr += sizeof(residentAttrHdr);

		fileName filenameattr;
		memcpy_s(static_cast<void*>(&filenameattr), sizeof(fileName), Buf + curPtr, sizeof(fileName));
		this->curPtr += sizeof(fileName);

		filename = new wchar_t[filenameattr.lenName + 1];
		memcpy_s(static_cast<void*>(filename), sizeof(wchar_t) * filenameattr.lenName,
			Buf + curPtr, sizeof(wchar_t) * filenameattr.lenName);
		filename[filenameattr.lenName] = '\0';

		cout << endl << "<<<<<<<<<<<<<<<<<<<<<<<<  FILENAME >>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		cout << "Flags : " << filenameattr.flags << endl;
		printf("Name length : %d\n", filenameattr.lenName);
		printf("Name space : %d\n", filenameattr.nameSpace);
		wcout << "file Name : " << filename << endl;

		delete[] filename;
	}
private:
	union {
		U8 Buf[1024];
		MFTHeader mftHdr;
	};
	U32 curPtr;
	U32 mftNum;
	wchar_t * filename;
};

void errorMsg(string error);
void hexdump(void * buf, U32 size);
U32 getHDD(U8 * HDDdump, U64 offset, U32 size);
U32 BIOSUEFI(U8 * HDDdump, vector<U32> & PartitionType);
U64 setVBR(U8 * HDDdump, VBR &vbr);