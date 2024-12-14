#include "te_hash.hpp"

#include <unordered_map>
#include <format>
#include <array>

namespace aurora {
    uint32_t hash(unsigned char const* array, unsigned int size) {
        uint32_t h = 0x811c9dc5;

        while (size > 0) {
            size--;
            h = (h ^ *array) * 0x1000193;
            array++;
        }

        h *= 0x2001;
        h = (h ^ (h >> 0x7)) * 0x9;
        h = (h ^ (h >> 0x11)) * 0x21;

        return h;
    }

    uint32_t hash(std::string_view str) {
        return hash((unsigned char const*)str.data(), static_cast<unsigned int>(str.size()));
    }

    namespace {
        bool gFilled = false;
        std::unordered_map<uint32_t, std::string> gHashtable;
    }

    void fill_hashtable() {
        std::array inputs = {
            "layer_volume",
            "pitch",
            "roll",
            "turn",
            "turn_auto",
            "scale_x",
            "scale_y",
            "scale_z",
            "offset_x",
            "offset_y",
            "offset_z",
            "scale_offset_y",
            "tile_phase",
            "path_phase",
            "intensity_step",
            "visibla01",
            "visibla02",
            "visible",
            "visiblz01",
            "visiblz02",
            "sequin_speed",
            "win",
            "win_checkpoint",
            "win_checkpoint_silent",
            "play",
            "play_clean",
            "pause",
            "resume",
            "stop",
            "emissive_color",
            "ambient_color",
            "diffuse_color",
            "specular_color",
            "reflectivity_color",
            "alpha",
            "frame",
            "thump_rails.a01",
            "thump_rails.a02",
            "thump_rails.ent",
            "thump_rails.z01",
            "thump_rails.z02",
            "thump_checkpoint.ent",
            "thump_rails_fast_activat.ent",
            "thump_boss_bonus.ent",
            "grindable_still.ent",
            "left_multi.a01",
            "left_multi.a02",
            "left_multi.ent",
            "left_multi.z01",
            "center_multi.a02",
            "center_multi.ent",
            "center_multi.z01",
            "right_multi.a02",
            "right_multi.ent",
            "right_multi.z01",
            "right_multi.z02",
            "grindable_quarters.ent",
            "grindable_double.ent",
            "grindable_thirds.ent",
            "grindable_with_thump.ent",
            "ducker_crak.ent",
            "jumper_1_step.ent",
            "jumper_boss.ent",
            "jumper_6_step.ent",
            "jump_high.ent",
            "jump_high_2.ent",
            "jump_high_4.ent",
            "jump_high_6.ent",
            "jump_boss.ent",
            "swerve_off.a01",
            "swerve_off.a02",
            "swerve_off.ent",
            "swerve_off.z01",
            "swerve_off.z02",
            "millipede_half.a01",
            "millipede_half.a02",
            "millipede_half.ent",
            "millipede_half.z01",
            "millipede_half.z02",
            "millipede_half_phrase.a01",
            "millipede_half_phrase.a02",
            "millipede_half_phrase.ent",
            "millipede_half_phrase.z01",
            "millipede_half_phrase.z02",
            "millipede_quarter.a01",
            "millipede_quarter.a02",
            "millipede_quarter.ent",
            "millipede_quarter.z01",
            "millipede_quarter.z02",
            "millipede_quarter_phrase.a01",
            "millipede_quarter_phrase.a02",
            "millipede_quarter_phrase.ent",
            "millipede_quarter_phrase.z01",
            "millipede_quarter_phrase.z02",
            "sentry.ent",
            "level_9.ent",
            "level_5.ent",
            "level_8.ent",
            "sentry_boss.ent",
            "level_7.ent",
            "level_6.ent",
            "sentry_boss_multilane.ent",
            "level_8_multi.ent",
            "level_9_multi.ent",
            "trees.ent",
            "trees_16.ent",
            "trees_4.ent",
            "speed_streaks_short.ent",
            "speed_streaks_RGB.ent",
            "smoke.ent",
            "death_shatter.ent",
            "speed_streaks.ent",
            "data_streaks_radial.ent",
            "boss_7_tunnel_enter.ent",
            "boss_damage_stage4.ent",
            "crakhed_damage.ent",
            "win_debris.ent",
            "crakhed_destroy.ent",
            "stalactites.ent",
            "aurora.ent",
            "vortex_decorator.ent",
            "boss_damage_stage3.ent",
            "boss_damage_stage1.ent",
            "boss_damage_stage2.ent",
            "black",
            "crakhed",
            "dark_blue",
            "dark_green",
            "dark_red",
            "light_blue",
            "light_green",
            "light_red",
            "fire",
            "diss11",
            "french12",
            "tutorial_thumps.ent",
            "boss_gate_pellet.ent",
            "rise1",
            "rise2",
            "rise3",
            "rise4",
            "rise5",
            "rise6",
            "rise7",
            "rise8",
            "rise9",
            "rise10",
            "speed",
            "color1",
            "color2",
            "color3",
            "color4",
            "color5",
            "color6",
            "color7",
            "color8",
            "color9",
            "color10",
            "jumper_1_step.ent",
            "jumper_6_step.ent",
            "jump_high_2.ent",
            "jump_high_4.ent",
            "jump_high_6.ent",
            "jump_high.ent",
            "trees.ent",
            "trees_16.ent",
            "trees_4.ent",
            "spike.ent",
            "spike_left.ent",
            "spike_right_base.ent",
            "spike_right.ent",
            "spike_right_ground_collide.ent",
            "boss_shield_tutorial.ent",
            "boss_shield.ent",
            "boss_shield_2.ent",
            "boss_shield_3.ent",
            "fingers1.ent",
            "fingers1_unison.ent",
            "fingers1_wave_in.ent",
            "fingers1_gate.ent",
            "fingers1_short.ent",
            "tentacle_decorative.ent",
            "fingers1_gate_8.ent",
            "fingers1_long.ent",
            "fingers1_long_20.ent",
            "diss1",
            "diss2",
            "diss3",
            "diss4",
            "diss5",
            "diss6",
            "diss7",
            "diss8",
            "diss9",
            "diss10",
            "diss11",
            "diss12",
            "diss13",
            "diss14",
            "diss15",
            "diss16",
            "diss17",
            "diss18",
            "diss19",
            "diss20",
            "turn_left_isolated.ent",
            "turn_left_start.ent",
            "turn_left_middle.ent",
            "turn_left_end.ent",
            "turn_right_isolated.ent",
            "turn_right_start.ent",
            "turn_right_middle.ent",
            "turn_right_end.ent",
            "start_power",
            "invulerability_beats",
            "base_bpm",
            "grind_remain_beats",
            "respawn",
            "debug_speed",
            "Gameplay",
            "grinding",
            "pounding",
            "invulnerable",
            "collide.ent",
            "weapon.ent",
            "boss_gate_pellet.ent",
            "fade",
            "r_gamma",
            "g_gamma",
            "b_gamma",
            "r_white",
            "g_white",
            "b_white",
            "r_output_white",
            "g_output_white",
            "b_output_white",
            "r_black",
            "g_black",
            "b_black",
            "r_output_black",
            "g_output_black",
            "b_output_black",
            "rgb_white",
            "rgb_black",
            "rgb_output_white",
            "rgb_output_black",
            "radial_blur",
            "stick_left_x"
        };

        for (auto& input : inputs) {
            gHashtable[hash(input)] = input;
        }

        // Unknown inputs to these hashes
        gHashtable[0x5232f8f9] = "*.anim Objects";
        gHashtable[0x7dd6b7d8] = "*.bend Objects";
        gHashtable[0x570e17fa] = "*.bind Objects";
        gHashtable[0x8f86650f] = "*.cam Objects";
        gHashtable[0xadb02913] = "*.ch Objects";
        gHashtable[0x4945e860] = "*.cond Objects";
        gHashtable[0xac1abb2c] = "*.dch Objects";
        gHashtable[0x9ce604da] = "*.dec Objects";
        gHashtable[0xacc2033e] = "*.dsp Objects";
        gHashtable[0xeae6beee] = "*.ent Objects";
        gHashtable[0x3bbcc4ec] = "*.env Objects";
        gHashtable[0x86621b1e] = "*.flow Objects";
        gHashtable[0x6222e06f] = "*.flt Objects";
        gHashtable[0x993811f5] = "*.flt Objects";
        gHashtable[0xc2fd0a11] = "*.gameplay Objects";
        gHashtable[0xaa63a508] = "*.gate Objects";
        gHashtable[0xc2aaec43] = "*.grp Objects";
        gHashtable[0xce7e85f6] = "*.leaf Objects";
        gHashtable[0x711a2715] = "*.light Objects";
        gHashtable[0xbcd17473] = "*.lvl Objects";
        gHashtable[0x490780b9] = "*.master Objects";
        gHashtable[0x1a5812f6] = "*.mastering Objects";
        gHashtable[0x7ba5c8e0] = "*.mat Objects";
        gHashtable[0xbf69f115] = "*.mesh Objects";
        gHashtable[0x1ba51443] = "*GFX.objlib Objects";
        gHashtable[0xb0954548] = "*Sequin.objlib Objects";
        gHashtable[0x9d1c6219] = "*Obj.objlib Objects";
        gHashtable[0x0b374d9e] = "*Level.objlib Objects";
        gHashtable[0xe674624f] = "*Avatar.objlib Objects";
        gHashtable[0x4890a3f6] = "*.path Objects";
        gHashtable[0x745dd78b] = "*.playspace Objects";
        gHashtable[0x230da622] = "*.pulse Objects";
        gHashtable[0x7aa8f390] = "*.samp Objects";
        gHashtable[0xd3058b5d] = "*.sdraw / .drawer Objects";
        gHashtable[0xcac934cf] = "*.sh Objects";
        gHashtable[0xd897d5db] = "*.spn Objects";
        gHashtable[0xd955fdc6] = "*.st Objects";
        gHashtable[0xe7b3aadb] = "*.steer Objects";
        gHashtable[0x96ba8a70] = "*.tex Objects";
        gHashtable[0x799c45a7] = "*.vib Objects";
        gHashtable[0x4f37349d] = "*.vr_settings Objects";
        gHashtable[0x7d9db5ef] = "*.xfm / .xfmer Objects";
    }

	std::string rev_hash(uint32_t hash) {
        if (!gFilled) {
            fill_hashtable();
            gFilled = true;
        }

        auto it = gHashtable.find(hash);
        if (it == gHashtable.end()) return std::format("{:x}", hash);
        return it->second;
	}
}