#include "IKRenderable.h"
#include "KEnvironment.h"
#include "File.h"
#include "CKLogic.h"
#include "CKGraphical.h"

void IKRenderable::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	if (kenv->version == KEnvironment::KVERSION_XXL1)
		return;
	if (kenv->version >= kenv->KVERSION_SPYRO)
		spUnk1 = file->readUint32();
	else
		anotherRenderable = kenv->readObjRef<IKRenderable>(file);
	lightSet = kenv->readObjRef<CLightSet>(file);
	flags = file->readUint32();
}

void IKRenderable::serialize(KEnvironment* kenv, File* file)
{
	if (kenv->version == KEnvironment::KVERSION_XXL1)
		return;
	if (kenv->version >= kenv->KVERSION_SPYRO)
		file->writeUint32(spUnk1);
	else
		kenv->writeObjRef(file, anotherRenderable);
	kenv->writeObjRef(file, lightSet);
	file->writeUint32(flags);
}

void IKRenderable::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	if (kenv->version == KEnvironment::KVERSION_XXL1)
		return;
	if (kenv->version >= kenv->KVERSION_SPYRO)
		r.reflect(spUnk1, "spUnk1");
	else
		r.reflect(anotherRenderable, "anotherRenderable");
	r.reflect(lightSet, "lightSet");
	r.reflect(flags, "flags");
}
