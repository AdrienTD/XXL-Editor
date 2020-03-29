#pragma once

struct KEnvironment;

RwClump * LoadDFF(const char * filename);
void AddTexture(KEnvironment & kenv, const char * filename);
