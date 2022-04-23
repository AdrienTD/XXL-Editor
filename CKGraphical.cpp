#include "CKGraphical.h"
#include "File.h"
#include "KEnvironment.h"
#include "rw.h"
#include "CKNode.h"

void CCloneManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t start = file->tell();

	if (kenv->version >= kenv->KVERSION_XXL2) {
		if (kenv->version >= kenv->KVERSION_SPYRO)
			sp_unk1 = file->readUint32();
		else
			x2_unkobj1 = kenv->readObjRef<CKObject>(file);
		x2_lightSet = kenv->readObjRef<CKObject>(file);
		x2_flags = file->readUint32();
	}

	_numClones = file->readUint32();
	if (_numClones == 0)
		return;
	_unk1 = file->readUint32();
	_unk2 = file->readUint32();
	_unk3 = file->readUint32();
	_unk4 = file->readUint32();

	if (kenv->version >= kenv->KVERSION_XXL2) {
		auto lightSet = kenv->readObjRef<CKObject>(file);
		assert(x2_lightSet == lightSet);
	}

	_clones.reserve(_numClones);
	for (uint32_t i = 0; i < _numClones; i++)
		_clones.push_back(kenv->readObjRef<CSGBranch>(file));

	rwCheckHeader(file, 0x22);
	_teamDict.deserialize(file);

	printf("Team at %X\n", file->tell() - start);
	rwCheckHeader(file, 0x1C);
	_team.deserialize(file);

	flinfos.reserve(_numClones);
	for (uint32_t i = 0; i < _numClones; i++) {
		std::array<float, 4> arr;
		for (float &f : arr)
			f = file->readFloat();
		flinfos.push_back(std::move(arr));
	}
}

void CCloneManager::serialize(KEnvironment * kenv, File * file)
{
	printf("CCloneManager at %08X\n", file->tell());

	if (kenv->version >= kenv->KVERSION_XXL2) {
		if (kenv->version >= kenv->KVERSION_SPYRO)
			file->writeUint32(sp_unk1);
		else
			kenv->writeObjRef(file, x2_unkobj1);
		kenv->writeObjRef(file, x2_lightSet);
		file->writeUint32(x2_flags);
	}

	file->writeUint32(_numClones);
	if (_numClones == 0)
		return;
	file->writeUint32(_unk1);
	file->writeUint32(_unk2);
	file->writeUint32(_unk3);
	file->writeUint32(_unk4);
	if (kenv->version >= kenv->KVERSION_XXL2)
		kenv->writeObjRef(file, x2_lightSet);
	for (auto &clone : _clones)
		kenv->writeObjRef(file, clone);
	_teamDict.serialize(file);
	_team.serialize(file);
	for (auto &fli : flinfos) {
		for (float &f : fli)
			file->writeFloat(f);
	}
}

void CManager2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version >= kenv->KVERSION_XXL2) {
		if (kenv->version >= kenv->KVERSION_SPYRO)
			spmg2dUnk0 = file->readUint32();
		else
			x2mg2dUnk1 = kenv->readObjRef<CKObject>(file);
		x2mg2dUnk2 = kenv->readObjRef<CKObject>(file);
		x2mg2dUnk3 = file->readUint32();
	}
	menuManager = kenv->readObjRef<CMenuManager>(file);
	scene1 = kenv->readObjRef<CScene2d>(file);
	scene2 = kenv->readObjRef<CScene2d>(file);
	if (kenv->version >= kenv->KVERSION_XXL2) {
		x2scene3 = kenv->readObjRef<CScene2d>(file);
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		ogscene4 = kenv->readObjRef<CScene2d>(file);
	}
}

void CManager2d::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version >= kenv->KVERSION_XXL2) {
		if (kenv->version >= kenv->KVERSION_SPYRO)
			file->writeUint32(spmg2dUnk0);
		else
			kenv->writeObjRef(file, x2mg2dUnk1);
		kenv->writeObjRef(file, x2mg2dUnk2);
		file->writeUint32(x2mg2dUnk3);
	}
	kenv->writeObjRef(file, menuManager);
	kenv->writeObjRef(file, scene1);
	kenv->writeObjRef(file, scene2);
	if (kenv->version >= kenv->KVERSION_XXL2) {
		kenv->writeObjRef(file, x2scene3);
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		kenv->writeObjRef(file, ogscene4);
	}
}

void CManager2d::deserializeGlobal(KEnvironment* kenv, File* file, size_t length)
{
	numFonts = file->readUint32();
}

void CMenuManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	scene = kenv->readObjRef<CScene2d>(file);
	messageBox = kenv->readObjRef<CMessageBox2d>(file);
	cmm2dVal = file->readUint8();
}

void CMenuManager::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, scene);
	kenv->writeObjRef(file, messageBox);
	file->writeUint8(cmm2dVal);
}

void CScene2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	first = kenv->readObjRef<CElement2d>(file);
	last = kenv->readObjRef<CElement2d>(file);
	numElements = file->readUint32();
	cs2dVal = file->readUint8();
}

void CScene2d::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, first);
	kenv->writeObjRef(file, last);
	file->writeUint32(numElements);
	file->writeUint8(cs2dVal);
}

void CElement2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	e2dUnk1 = file->readUint8();
	e2dUnk2 = file->readUint8();
	e2dUnk3 = file->readUint8();
	e2dUnk4 = file->readUint8();
	scene = kenv->readObjRef<CScene2d>(file);
	previous = kenv->readObjRef<CElement2d>(file);
	next = kenv->readObjRef<CElement2d>(file);
	e2dUnk5 = file->readFloat();
	e2dUnk6 = file->readFloat();
	e2dUnk7 = file->readFloat();
}

void CElement2d::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint8(e2dUnk1);
	file->writeUint8(e2dUnk2);
	file->writeUint8(e2dUnk3);
	file->writeUint8(e2dUnk4);
	kenv->writeObjRef(file, scene);
	kenv->writeObjRef(file, previous);
	kenv->writeObjRef(file, next);
	file->writeFloat(e2dUnk5);
	file->writeFloat(e2dUnk6);
	file->writeFloat(e2dUnk7);
}

void CContainer2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CElement2d::deserialize(kenv, file, length);
	name = file->readSizedString<uint16_t>();
	scene1 = kenv->readObjRef<CScene2d>(file);
	scene2 = kenv->readObjRef<CScene2d>(file);
	cctr2dVal = file->readUint8();
}

void CContainer2d::serialize(KEnvironment * kenv, File * file)
{
	CElement2d::serialize(kenv, file);
	file->writeSizedString<uint16_t>(name);
	kenv->writeObjRef(file, scene1);
	kenv->writeObjRef(file, scene2);
	file->writeUint8(cctr2dVal);
}

void CMessageBox2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CElement2d::deserialize(kenv, file, length);
	container = kenv->readObjRef<CContainer2d>(file);
	msgboxUnk = file->readUint32();
	text = kenv->readObjRef<CText2d>(file);
	billboard = kenv->readObjRef<CBillboard2d>(file);
	button1 = kenv->readObjRef<CColorTextButton2d>(file);
	button2 = kenv->readObjRef<CColorTextButton2d>(file);
	button3 = kenv->readObjRef<CColorTextButton2d>(file);
}

void CMessageBox2d::serialize(KEnvironment * kenv, File * file)
{
	CElement2d::serialize(kenv, file);
	kenv->writeObjRef(file, container);
	file->writeUint32(msgboxUnk);
	kenv->writeObjRef(file, text);
	kenv->writeObjRef(file, billboard);
	kenv->writeObjRef(file, button1);
	kenv->writeObjRef(file, button2);
	kenv->writeObjRef(file, button3);
}

void CText2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CElement2d::deserialize(kenv, file, length);
	locManager = kenv->readObjRef<CKObject>(file);
	ce2dUnk1 = file->readFloat();
	ce2dUnk2 = file->readFloat();
	ce2dUnk3 = file->readUint32();
	ce2dUnk4 = file->readUint32();
	text.resize(file->readUint32());
	file->read((wchar_t*)text.data(), text.size() * 2);
	ce2dUnk5 = file->readUint32();
	rwCheckHeader(file, 0x1A2);
	brush.deserialize(file);
	for (auto &u : ce2dUnk6)
		u = file->readUint32();
	ce2dUnk7 = file->readUint8();
	ce2dUnk8 = file->readUint8();
	ce2dUnk9 = file->readUint8();
	ce2dUnkA = file->readFloat();
}

void CText2d::serialize(KEnvironment * kenv, File * file)
{
	CElement2d::serialize(kenv, file);
	kenv->writeObjRef(file, locManager);
	file->writeFloat(ce2dUnk1);
	file->writeFloat(ce2dUnk2);
	file->writeUint32(ce2dUnk3);
	file->writeUint32(ce2dUnk4);
	file->writeUint32(text.size());
	file->write(text.data(), text.size() * 2);
	file->writeUint32(ce2dUnk5);
	brush.serialize(file);
	for (auto &u : ce2dUnk6)
		file->writeUint32(u);
	file->writeUint8(ce2dUnk7);
	file->writeUint8(ce2dUnk8);
	file->writeUint8(ce2dUnk9);
	file->writeFloat(ce2dUnkA);
}

void CColorTextButton2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CText2d::deserialize(kenv, file, length);
	ccolor = file->readUint32();
}

void CColorTextButton2d::serialize(KEnvironment * kenv, File * file)
{
	CText2d::serialize(kenv, file);
	file->writeUint32(ccolor);
}

void CBillboard2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CElement2d::deserialize(kenv, file, length);
	rwCheckHeader(file, 0x1A2);
	brush.deserialize(file);
	texture = file->readSizedString<uint16_t>();
	color1 = file->readUint32();
	color2 = file->readUint32();
	flt1 = file->readFloat();
	flt2 = file->readFloat();
	bb1 = file->readUint8();
	bb2 = file->readUint8();
	for (float &f : fltarr)
		f = file->readFloat();
}

void CBillboard2d::serialize(KEnvironment * kenv, File * file)
{
	CElement2d::serialize(kenv, file);
	brush.serialize(file);
	file->writeSizedString<uint16_t>(texture);
	file->writeUint32(color1);
	file->writeUint32(color2);
	file->writeFloat(flt1);
	file->writeFloat(flt2);
	file->writeUint8(bb1);
	file->writeUint8(bb2);
	for (float& f : fltarr)
		file->writeFloat(f);
}

void CAnimationManager::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		commonAnims.deserialize(kenv, file, length);
	}
	else {
		auto numSectors = file->readUint32();
		arSectors.resize(numSectors);
		for (int i = 0; i < (int)numSectors; i++)
			arSectors[i].read(file);
	}
}

void CAnimationManager::serialize(KEnvironment* kenv, File* file)
{
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		commonAnims.serialize(kenv, file);
	}
	else {
		file->writeUint32(arSectors.size());
		for (auto& str : arSectors)
			str.write(kenv, file);
	}
}

void CAnimationManager::onLevelLoaded(KEnvironment* kenv)
{
	// Assuming 1st ref for common sector, 2nd ref for sector 0, 3rd ref for sector 1, etc. 
	for (int i = 0; i < (int)arSectors.size(); i++)
		arSectors[i].bind(kenv, i - 1);
}

void CSectorAnimation::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	anims.resize(file->readUint32());
	for (auto& anim : anims) {
		rwCheckHeader(file, 0x1B);
		anim.rwAnim.deserialize(file);
		if (kenv->version >= kenv->KVERSION_XXL2)
			for (float& f : anim.x2AnimVals)
				f = file->readFloat();
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			for (auto& u : anim.arAnimValues)
				u = file->readUint32();
	}
}

void CSectorAnimation::serialize(KEnvironment* kenv, File* file)
{
	file->writeUint32(anims.size());
	for (auto& anim : anims) {
		anim.rwAnim.serialize(file);
		if (kenv->version >= kenv->KVERSION_XXL2)
			for (float& f : anim.x2AnimVals)
				file->writeFloat(f);
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			for (auto& u : anim.arAnimValues)
				file->writeUint32(u);
	}
}

void CBillboard2dList::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CElement2d::deserialize(kenv, file, length);
	cblBills.resize(file->readUint32());
	for (Bill& bill : cblBills) {
		bill.cblUnk1 = (int32_t)file->readUint32();
		rwCheckHeader(file, 0x1A2);
		bill.brush.deserialize(file);
		bill.cblUnk4 = file->readUint8();
		for (float& f : bill.cblUnk5)
			f = file->readFloat();
	}
	cblUnk10 = file->readUint32();
	cblTextures.resize(file->readUint32());
	for (auto& tex : cblTextures)
		tex = file->readSizedString<uint16_t>();
	cblUnk33[0] = file->readFloat();
	cblUnk33[1] = file->readFloat();
	cblUnk34 = file->readUint8();
	cblUnk35 = file->readFloat();
}

void CBillboard2dList::serialize(KEnvironment* kenv, File* file)
{
	CElement2d::serialize(kenv, file);
	file->writeUint32(cblBills.size());
	for (Bill& bill : cblBills) {
		file->writeUint32((uint32_t)bill.cblUnk1);
		bill.brush.serialize(file);
		file->writeUint8(bill.cblUnk4);
		for (float& f : bill.cblUnk5)
			file->writeFloat(f);
	}
	file->writeUint32(cblUnk10);
	file->writeUint32(cblTextures.size());
	for (auto& tex : cblTextures)
		file->writeSizedString<uint16_t>(tex);
	file->writeFloat(cblUnk33[0]);
	file->writeFloat(cblUnk33[1]);
	file->writeUint8(cblUnk34);
	file->writeFloat(cblUnk35);
}

void CBillboardButton2d::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CBillboard2d::deserialize(kenv, file, length);
	billButtonColor = file->readUint32();
}

void CBillboardButton2d::serialize(KEnvironment* kenv, File* file)
{
	CBillboard2d::serialize(kenv, file);
	file->writeUint32(billButtonColor);
}
