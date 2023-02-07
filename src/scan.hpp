#pragma once

#include <mutex>
#include <vector>
#include <filesystem>
#include <ebur128.h>

void free_ebur128(ebur128_state *ebur128);

enum class FileType {
    INVALID = -1,
    DEFAULT,
    MP2,
    MP3,
    FLAC,
    OGG,
    OPUS,
    M4A,
    WMA,
    WAV,
    AIFF,
    WAVPACK,
    APE,
	MPC
};

struct ScanResult {
	double track_gain;
	double track_peak;
	double track_loudness;

	double album_gain;
	double album_peak;
	double album_loudness;
};

struct ScanData {
    unsigned int files = 0;
	unsigned int skipped = 0;
    unsigned int clipping_adjustments = 0;
    double total_gain = 0.0;
    double total_peak = 0.0;
    unsigned int total_negative = 0;
    unsigned int total_positive = 0;
    std::vector<std::string> error_directories;
};


class ScanJob {
	public:
		struct Track {
			std::string path;
			FileType type;
			std::string container;
			ScanResult result;
			int codec_id;
			std::unique_ptr<ebur128_state, decltype(&free_ebur128)> ebur128;
			bool tclip = false;
			bool aclip = false;

			Track(const std::string &path, FileType type) : path(path), type(type), ebur128(nullptr, free_ebur128) {};
			bool scan(const Config &config, std::mutex *ffmpeg_mutex);
			bool calculate_loudness(const Config &config);
		};

		FileType type;
		const Config &config;
		std::string path;
		int nb_files;
		bool error = false;
		int clipping_adjustments = 0;
		unsigned int skipped = 0;

		ScanJob(const std::string &path, std::vector<Track> &&tracks, const Config &config) : path(path), tracks(std::move(tracks)), nb_files(tracks.size()), config(config) {}
		static ScanJob* factory(char **files, int nb_files, const Config &config);
		static ScanJob* factory(const std::filesystem::path &path);
		bool scan(std::mutex *ffmpeg_mutex = nullptr);
		void update_data(ScanData &data);

	private:
		std::vector<Track> tracks;

		void calculate_loudness();
		void calculate_album_loudness();
		void tag_tracks();
};
