#pragma once

struct KEnvironment;
struct CKObject;
struct CKBoundingShape;

namespace NodeCollisionUtils
{
	void LoadNodeCollisionInfo(int gameVersion);
	void CreateCollisionsForObject(KEnvironment& kenv, CKObject* owner);
}