#pragma once
#include "../surface_provider_node.h"

// idname = Noise
class NoiseSPN : public SurfaceProviderNode
{
public:

	enum NoiseType
	{
		PERLIN,
		PERLIN_SEED,
		RIDGE,
		FBM,
		TURBULENCE,

		END_MARKER
	};

	NoiseType noise_type;

	static constexpr int INPUT_X			= 0;
	static constexpr int INPUT_LACUNARITY	= 1;
	static constexpr int INPUT_GAIN			= 2;
	static constexpr int INPUT_OFFSET		= 3;
	static constexpr int OUTPUT				= 4;

	float val_lacunarity;
	float val_gain;
	float val_offset;
	float in_scale;
	float out_scale;
	int octaves;
	int seed;
	int wrap_pow[3];
	int wrap[3];

	virtual bool do_imgui(int id);
	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};