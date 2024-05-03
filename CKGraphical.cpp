#include "CKGraphical.h"
#include "File.h"
#include "KEnvironment.h"
#include "rw.h"
#include "CKNode.h"
#include "CKLogic.h"

void CCloneManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t start = file->tell();

	IKRenderable::deserialize(kenv, file, length);

	_numClones = file->readUint32();
	if (_numClones == 0)
		return;
	_unk1 = file->readUint32();
	_unk2 = file->readUint32();
	_unk3 = file->readUint32();
	_unk4 = file->readUint32();

	if (kenv->version >= kenv->KVERSION_XXL2) {
		auto dupLightSet = kenv->readObjRef<CKObject>(file);
		assert(lightSet == dupLightSet);
	}

	_clones.reserve(_numClones);
	for (uint32_t i = 0; i < _numClones; i++)
		_clones.push_back(kenv->readObjRef<CSGBranch>(file));

	rwCheckHeader(file, 0x22);
	_teamDict.deserialize(file);

	//printf("Team at %X\n", file->tell() - start);
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
	//printf("CCloneManager at %08X\n", file->tell());

	IKRenderable::serialize(kenv, file);

	file->writeUint32(_numClones);
	if (_numClones == 0)
		return;
	file->writeUint32(_unk1);
	file->writeUint32(_unk2);
	file->writeUint32(_unk3);
	file->writeUint32(_unk4);
	if (kenv->version >= kenv->KVERSION_XXL2)
		kenv->writeObjRef(file, lightSet);
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
	IKRenderable::deserialize(kenv, file, length);
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
	IKRenderable::serialize(kenv, file);
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
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		IKRenderable::deserialize(kenv, file, length);
	first = kenv->readObjRef<CElement2d>(file);
	last = kenv->readObjRef<CElement2d>(file);
	numElements = file->readUint32();
	cs2dVal = file->readUint8();
}

void CScene2d::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		IKRenderable::serialize(kenv, file);
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
	if (kenv->version < KEnvironment::KVERSION_OLYMPIC)
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
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		ogLocTextAccessor = kenv->readObjRef<CKObject>(file);
	}
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		ogText2d_1 = kenv->readObjRef<CKObject>(file);
		ogText2d_2 = kenv->readObjRef<CKObject>(file);
	}
}

void CText2d::serialize(KEnvironment * kenv, File * file)
{
	CElement2d::serialize(kenv, file);
	kenv->writeObjRef(file, locManager);
	file->writeFloat(ce2dUnk1);
	file->writeFloat(ce2dUnk2);
	file->writeUint32(ce2dUnk3);
	if (kenv->version < KEnvironment::KVERSION_OLYMPIC)
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
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		kenv->writeObjRef(file, ogLocTextAccessor);
	}
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		kenv->writeObjRef(file, ogText2d_1);
		kenv->writeObjRef(file, ogText2d_2);
	}
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
	if (kenv->version == KEnvironment::KVERSION_XXL2) {
		x2Byte = file->readUint8();
	}
	color1 = file->readUint32();
	color2 = file->readUint32();
	flt1 = file->readFloat();
	flt2 = file->readFloat();
	bb1 = file->readUint8();
	bb2 = file->readUint8();
	for (float &f : fltarr)
		f = file->readFloat();
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		arByte = file->readUint8();
		arUnkObject = kenv->readObjRef<CKObject>(file);
	}
	if (kenv->isUsingNewFilenames()) {
		for (auto& num : spIntArray)
			num = file->readInt32();
	}
}

void CBillboard2d::serialize(KEnvironment * kenv, File * file)
{
	CElement2d::serialize(kenv, file);
	brush.serialize(file);
	file->writeSizedString<uint16_t>(texture);
	if (kenv->version == KEnvironment::KVERSION_XXL2) {
		file->writeUint8(x2Byte);
	}
	file->writeUint32(color1);
	file->writeUint32(color2);
	file->writeFloat(flt1);
	file->writeFloat(flt2);
	file->writeUint8(bb1);
	file->writeUint8(bb2);
	for (float& f : fltarr)
		file->writeFloat(f);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		file->writeUint8(arByte);
		kenv->writeObjRef<CKObject>(file, arUnkObject);
	}
	if (kenv->isUsingNewFilenames()) {
		for (auto& num : spIntArray)
			file->writeInt32(num);
	}
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

int32_t CAnimationManager::addAnimation(RwAnimAnimation& rwAnim, int sectorIndex)
{
	// TODO: Reuse animation that was already added (use hashes), sectors
	CSectorAnimation* secAnim = &commonAnims;
	int32_t index = (int32_t)secAnim->anims.size();
	CSectorAnimation::Animation& anim = secAnim->anims.emplace_back();
	anim.rwAnim = std::move(rwAnim);
	return index;
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

void CLightSet::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		ogSector = file->readInt32();
	clsUnk0 = file->readInt32();
	bbox.deserialize(file);
	sceneNode.read(file);
	if (kenv->version >= KEnvironment::KVERSION_SPYRO)
		spUnkInt = file->readInt32();
	for (auto& ref : lightComponents)
		ref = kenv->readObjRef<CKObject>(file);
}

void CLightSet::serialize(KEnvironment* kenv, File* file)
{
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		file->writeInt32(ogSector);
	file->writeInt32(clsUnk0);
	bbox.serialize(file);
	sceneNode.write(kenv, file);
	if (kenv->version >= KEnvironment::KVERSION_SPYRO)
		file->writeInt32(spUnkInt);
	for (auto& ref : lightComponents)
		kenv->writeObjRef(file, ref);
}

void CLightSet::onLevelLoaded(KEnvironment* kenv)
{
	// TODO: XXL2
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		sceneNode.bind(kenv, ogSector - 1);
}

void CLightManager::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	IKRenderable::deserialize(kenv, file, length);
	lightSets.resize(file->readUint32());
	for (auto& ref : lightSets)
		ref = kenv->readObjRef<CLightSet>(file);
	if (kenv->version >= KEnvironment::KVERSION_SPYRO) {
		spHDStuff.resize(file->readUint32());
		for (auto& ref : spHDStuff)
			ref = kenv->readObjRef<CKObject>(file);
	}
}

void CLightManager::serialize(KEnvironment* kenv, File* file)
{
	IKRenderable::serialize(kenv, file);
	file->writeUint32(lightSets.size());
	for (auto& ref : lightSets)
		kenv->writeObjRef(file, ref);
	if (kenv->version >= KEnvironment::KVERSION_SPYRO) {
		file->writeUint32(spHDStuff.size());
		for (auto& ref : spHDStuff)
			kenv->writeObjRef(file, ref);
	}
}

void CBlurData::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(cbdUnk0, "cbdUnk0");
	r.reflect(cbdUnk1, "cbdUnk1");
	r.reflect(cbdUnk2, "cbdUnk2");
	r.reflect(cbdUnk3, "cbdUnk3");
	r.reflect(cbdUnk4, "cbdUnk4");
	r.reflect(cbdUnk5, "cbdUnk5");
	r.reflect(cbdUnk6, "cbdUnk6");
	r.reflect(cbdUnk7, "cbdUnk7");
	r.reflect(cbdUnk8, "cbdUnk8");
	r.reflect(cbdUnk9, "cbdUnk9");
	r.reflect(cbdUnk10, "cbdUnk10");
	r.reflect(cbdUnk11, "cbdUnk11");
	r.reflect(cbdUnk12, "cbdUnk12");
}

void CPostRenderingFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKReflectableRenderable::reflectMembers2(r, kenv);
	r.reflect(cprfUnk0, "cprfUnk0");
	r.reflect(cprfUnk1, "cprfUnk1");
	r.reflect(cprfUnk2, "cprfUnk2");
	r.reflect(cprfUnk3, "cprfUnk3");
}

void CHDRData::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(chdrdUnk0, "chdrdUnk0");
	r.reflect(chdrdUnk1, "chdrdUnk1");
	r.reflect(chdrdUnk2, "chdrdUnk2");
	r.reflect(chdrdUnk3, "chdrdUnk3");
};

void CKSpawnPoolParams::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	if (kenv->version < KEnvironment::KVERSION_OLYMPIC) {
		r.reflect(cksppUnk0, "cksppUnk0");
		r.reflect(cksppUnk1, "cksppUnk1");
		r.reflect(cksppUnk2, "cksppUnk2");
		r.reflect(cksppUnk3, "cksppUnk3");
		r.reflect(cksppUnk4, "cksppUnk4");
	}
	else {
		r.reflect(ogQuakeCpnt, "ogQuakeCpnt");
		r.reflect(ogFlags, "ogFlags");
		r.reflectSize<uint32_t>(ogQCUpdaters, "ogQCUpdaters_size");
		r.reflect(ogQCUpdaters, "ogQCUpdaters");
	}
};

void CBackgroundManager::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	IKRenderable::reflectMembers2(r, kenv);
	r.reflect(cbmUnk0, "cbmUnk0");
	cbmUnk1.resize(cbmUnk0);
	r.reflect(cbmUnk1, "cbmUnk1");
	if (kenv->version <= KEnvironment::KVERSION_XXL2) {
		r.reflect(cbmUnk2, "cbmUnk2");
		r.reflect(cbmUnk3, "cbmUnk3");
	}
}

void CBackgroundManager::onLevelLoaded(KEnvironment* kenv)
{
	for (size_t i = 0; i < cbmUnk1.size(); ++i)
		std::get<0>(cbmUnk1[i]).bind(kenv, i - 1);
}

void CKFlashManager::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflectSize<uint32_t>(ckfmFlashUI, "ckfmFlashUI_size");
	r.reflect(ckfmFlashUI, "ckfmFlashUI");
};

void CKFlashBase::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(ckfaUnk0, "ckfaUnk0");
	r.reflect(ckfaUnk1, "ckfaUnk1");
	r.reflect(ckfaUnk2, "ckfaUnk2");
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		r.reflect(ckfaOgUnk, "ckfaOgUnk");
}

void CKFlashAnimation::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKFlashBase::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(ckfaUnk5, "ckfaUnk5_size");
	r.reflect(ckfaUnk5, "ckfaUnk5");
};

void CKFlashText::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKFlashBase::reflectMembers2(r, kenv);
	r.reflect(ckftUnk3, "ckftUnk3");
	r.reflect(ckftUnk4, "ckftUnk4");
	r.reflect(ckftUnk5, "ckftUnk5");
	r.reflect(ckftUnk6, "ckftUnk6");
	r.reflect(ckftUnk7, "ckftUnk7");
};

void CKFlashMessageIn::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKFlashBase::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(ckfmiUnk1, "ckfmiUnk1_size");
	r.reflect(ckfmiUnk1, "ckfmiUnk1");
}

void CKFlashMessageOut::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKFlashBase::reflectMembers2(r, kenv);
}

void CSpawnManager::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	IKRenderable::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(spawnPools, "spawnPools_size");
	r.reflect(spawnPools, "spawnPools");
	r.reflectSize<uint32_t>(spParams, "spParams_size");
	r.reflect(spParams, "spParams");
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		r.reflectSize<uint32_t>(ogRenderParams, "ogRenderParams_size");
		r.reflect(ogRenderParams, "ogRenderParams");
		r.reflectSize<uint32_t>(ogNodes, "ogNodes_size");
		r.reflect(ogNodes, "ogNodes");
	}
}

void CKSpawnPool::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflectSize<uint32_t>(ckspUnk1, "ckspUnk1_size");
	r.reflect(ckspUnk1, "ckspUnk1");
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		r.reflectSize<uint32_t>(ogRenderParams, "ogRenderParams_size");
		r.reflect(ogRenderParams, "ogRenderParams");
		r.reflectSize<uint32_t>(ogNodes, "ogNodes_size");
		r.reflect(ogNodes, "ogNodes");
	}
	r.reflect(ckspUnk8, "ckspUnk8");
	if (kenv->version <= KEnvironment::KVERSION_XXL2) {
		r.reflectSize<uint32_t>(ckspUnk9, "ckspUnk9_size");
		r.onRead<CKSpawnPool>(this, [](File* file, CKSpawnPool* pool) -> void {
			for (auto& [numBones, frameList, fltValue] : pool->ckspUnk9) {
				numBones = file->readUint32();
				rwCheckHeader(file, 0xE);
				frameList = std::make_shared<RwFrameList>();
				frameList->deserialize(file);
				fltValue = file->readFloat();
			}
			});
		r.onWrite<CKSpawnPool>(this, [](File* file, CKSpawnPool* pool) -> void {
			for (auto& [numBones, frameList, fltValue] : pool->ckspUnk9) {
				file->writeUint32(numBones);
				frameList->serialize(file);
				file->writeFloat(fltValue);
			}
			});
	}
}

void CFlashHotSpot::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflectSize<uint32_t>(fhs2DList, "fhs2DList_size");
	r.reflect(fhs2DList, "fhs2DList");
	r.reflectSize<uint32_t>(fhs3DList, "fhs3DList_size");
	r.reflect(fhs3DList, "fhs3DList");
	r.reflect(cfhsUnk5, "cfhsUnk5");
	r.reflect(cfhsUnk6, "cfhsUnk6");
}

void CDistortionScreenData::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(cdsdUnk0, "cdsdUnk0");
	r.reflect(cdsdUnk1, "cdsdUnk1");
	r.reflect(cdsdUnk2, "cdsdUnk2");
	r.reflect(cdsdUnk3, "cdsdUnk3");
	r.reflect(cdsdUnk4, "cdsdUnk4");
	r.reflectSize<uint32_t>(cdsdUnk6, "cdsdUnk6_size");
	r.reflect(cdsdUnk6, "cdsdUnk6");
	r.reflect(cdsdUnk7, "cdsdUnk7");
}

void CFlashMessageBox2d::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	if (kenv->version <= KEnvironment::KVERSION_XXL2) {
		r.reflect(cfmbUnk0, "cfmbUnk0");
		r.reflect(cfmbUnk1, "cfmbUnk1");
		r.reflect(cfmbUnk2, "cfmbUnk2");
		r.reflect(cfmbUnk3, "cfmbUnk3");
		r.reflect(cfmbUnk4, "cfmbUnk4");
		r.reflect(cfmbUnk12, "cfmbUnk12");
		r.reflect(cfmbUnk17, "cfmbUnk17");
	}
	else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		r.reflect(ogMenuMessageBox, "ogMenuMessageBox");
	}
}

void CVideoManager::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	IKRenderable::deserialize(kenv, file, length);
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		ogColorizedScreenFx = kenv->readObjRef<CColorizedScreenFx>(file);
}

void CVideoManager::serialize(KEnvironment* kenv, File* file)
{
	IKRenderable::serialize(kenv, file);
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		kenv->writeObjRef(file, ogColorizedScreenFx);
}

void CVideoManager::deserializeGlobal(KEnvironment* kenv, File* file, size_t length)
{
	videos.resize(file->readUint32());
	for (auto& ref : videos)
		ref = kenv->readObjRef<CKObject>(file);
	vmFloat1 = file->readFloat();
	vmFloat2 = file->readFloat();
}

void CKPBuffer::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	IKRenderable::reflectMembers2(r, kenv);
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		r.reflect(ogString, "ogString");
}
