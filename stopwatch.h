#include <chrono>
#include <string>
#include <exception>
#include <vector>
#include <utility>
#include <set>
#include <fstream>
#include <cmath>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif


namespace Tazio {
	
	class Stopwatch {
		std::chrono::time_point<std::chrono::steady_clock> init_timepoint;
		
		// Output file CSV
		std::ofstream output_file;
		
		std::vector<double> running_mean;
		std::vector<double> running_variance;
		
		int current_iteration = 0;
		// Each time label corresponds to a list of measurements of different iterations
		std::vector<std::pair<std::string, std::vector<double> > > timestamps;
		std::vector<std::string> labels;
		bool find_label(std::string label);
		static std::string error_string() {return "ERROR: ";}
		int get_label_index(std::string label);
		void write_to_file();
	  public:
		void init();
		void end();
		void partial(std::string label);
		void mean_time(std::string label, double& mean, double& stddev);
		void set_output_file(std::string filename);
		void set_labels(const std::vector<std::string>& labels);
	};
	
	class StopwatchException : public std::exception {
			std::string error_msg_;
		public:
			explicit StopwatchException(std::string error_msg) : error_msg_(error_msg) {};
			const char* what() const throw() {
				return error_msg_.c_str();
			}
	};
}

void Tazio::Stopwatch::init() {
	init_timepoint = std::chrono::steady_clock::now();
}

void Tazio::Stopwatch::set_labels(const std::vector<std::string>& labels) {
	this->labels = labels;
	this->labels.push_back(std::string("end"));
	
	for (const auto& l : this->labels) {
		timestamps.push_back(std::make_pair(l, std::vector<double>()));
		running_mean.push_back(0.0);
		running_variance.push_back(0.0);
	}
}

void Tazio::Stopwatch::end() {
	partial(std::string("end"));
	
	write_to_file();
	current_iteration++;
}	

bool Tazio::Stopwatch::find_label(std::string label) {
	bool found = false;
	
	for (const auto& l : labels) {
		if (label == l)
			{ found = true; break;}
	}
	
	return found;
}

int Tazio::Stopwatch::get_label_index(std::string label) {
	int label_index = -1;
	int len;
	for (label_index = 0, len = timestamps.size(); label_index < len; label_index++) {
		if (timestamps[label_index].first == label) {
			break;
		}
	}
	
	return label_index;
}

void Tazio::Stopwatch::partial(std::string label) {	
    // Check if label was already assigned
	if (!find_label(label)) {
		throw StopwatchException(error_string() + "Label \"" + label + "\" was not registered with set_labels"); 
	}

	int label_index = get_label_index(label);

	double time_interval = double(std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now() - init_timepoint).count())*1E-9;
		
	timestamps[label_index].second.push_back(time_interval);
	
	double iterations = timestamps[label_index].second.size();
	
	double old_mean = running_mean[label_index];
	running_mean[label_index] = (1.0 - 1.0/iterations)*old_mean + 1.0/iterations*time_interval;
	
	if (iterations > 1)
		running_variance[label_index] = (iterations - 2)/(iterations - 1)*running_variance[label_index] + 1/iterations*std::pow(time_interval - old_mean, 2);
}

void Tazio::Stopwatch::set_output_file(std::string filename) {
	if (labels.size() == 0)
		throw StopwatchException(error_string() + "set_output_file must be called after set_labels");
	
	output_file.open(filename.c_str());
	if (!output_file.is_open())
		throw StopwatchException(error_string() + "Cannot open file: " + filename);		
		
	for (int i = 0; i < labels.size(); i++) {
		output_file << labels[i];
		if (i != labels.size()-1)
			output_file << ',';
	}
	
	output_file << std::endl;
}

void Tazio::Stopwatch::write_to_file() {
	int len;
	for (int i = 0, len = labels.size(); i < len; i++) {
		
		#ifdef DEBUG
		std::cout << "Current label " << labels[i] << std::endl;
		std::cout << "Labels " << timestamps.size() << std::endl;
		std::cout << "Iterations " << timestamps[i].second.size() << std::endl;
		#endif
		output_file << timestamps[i].second.back();
		if (i != len -1)
			output_file << ',';
	}
	
	output_file << std::endl;
}

void Tazio::Stopwatch::mean_time(std::string label, double& mean, double& stddev) {
	if (!find_label(label))
		throw StopwatchException(error_string() + "label \"" + label + "\" does not exist");
		
		
	double time = 0;
	int len;
	int label_index = get_label_index(label);
	for (label_index = 0, len = timestamps.size(); label_index < len; label_index++) {
		if (timestamps[label_index].first == label) {
			break;
		}
	}
	
	int iterations = timestamps[label_index].second.size();
	
	// Iterate over several measurements for this label
	for (int i = 0; i < iterations; i++) {
		time += timestamps[label_index].second[i];
	}
	
	// Return average
	mean = time/timestamps[label_index].second.size();
	
	if (iterations > 1) {
		stddev = 0;
		for (int i = 0; i < iterations; i++) {
			stddev += std::pow(timestamps[label_index].second[i] - mean, 2);
		}
		
		stddev /= (iterations-1);
		stddev = std::sqrt(stddev);
	}
	else {
		// Cannot calculate stddev with 1 measurement
		stddev = -1;
	}
}
