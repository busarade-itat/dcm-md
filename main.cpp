#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <list>
#include <map>

#include <signal.h>

#include <algorithm>
#include <boost/program_options.hpp>
#include <libltdl/lt_system.h>

#include "CDA.hh"
#include "Abstraction.hh"
#include "VerticalEventBase.hh"
#include "functions.hh"

CDA* cda = nullptr;

void stopHandler(int){

          if (cda != nullptr)
          	cda->stop = true;
}

using namespace boost;
namespace po = boost::program_options;

/**
 * 'Boilerplate' adapted from https://gist.github.com/ksimek/4a2814ba7d74f778bbee
 */
class Options
{
public:
	bool parse(int argc, char** argv)
	{
		std::vector<std::string> config_fnames;
		po::options_description general("General Options");
		general.add_options()
				("help", "Display this help message")
				("disc,d", po::value<std::string>(&gname),
				 "Extract discriminant chronicles using this file as negative dataset")
				("mincs", po::value<unsigned> (&mincs), "Minimum size of extracted chronicles")
				("maxcs", po::value<unsigned> (&maxcs), "Maximum size of extracted chronicles")
				("tid", "Output the tid list in addition of extracted chronicles (first line is 0) /!\\ has to be proven to really correspond to the right line numbers")
				("close,c", "Extract frequent closed chronicles or discriminant chronicles from closed multisets if --disc is used")
				("json,j", "Output format is json instead of plain text")
				("verbose,v", "The program will speak")
                ("vecsize,s", po::value<unsigned> (&vectorSize)->required(), "Number of vector components in the input file")
                ("gmin,g", po::value<float>(&gmin), "Minimal growth threshold\ndefault : 2")
				;

		po::options_description desc;
		desc.add(general);

		po::options_description hidden("Positional Options (required)");
		hidden.add_options()
				("input_file,i", po::value<std::string>(&fname)->required(),
				 "input file containing dataset to mine (string)\n"
						 "- positive dataset if --disc is used\n"
						 "positional : input_file")
				("fmin,f", po::value<float>(&fmin)->required(),
				 "minimal frequency threshold (number)\n"
						 "Number of sequences if >= 1 (support)\n"
						 "Percent of positive sequences number else\n"
						 "positional fmin")
				;

		po::positional_options_description p;
		p.add("input_file", 1);
		p.add("fmin", 1);

		po::variables_map vm;

		if (!notify(argc, argv, hidden, desc, p, vm)) return false;
		if(!parse_vm(vm)) return false;
		return parse_files(vm);
	}

private:
	bool notify(int argc, char** argv,
				const po::options_description& hidden,
				const po::options_description& desc,
				po::positional_options_description& p,
				po::variables_map& vm
	){
		po::options_description options;
		options.add(desc);
		options.add(hidden);
		bool need_help = false;

		try {
			po::store(po::command_line_parser(argc, (const char *const *) argv).
							  options(options).
							  positional(p).
							  run(),
					  vm);
		}
		catch(po::error &ex) {
			std::cout << ex.what() << std::endl << std::endl;
			need_help = true;
		}

		if (!need_help && !vm.count("help")) {
			try {
				po::notify(vm);
			}
			catch (po::error &ex) {
				std::cout << ex.what() << std::endl;
				need_help = true;
			}
		}
		else
			need_help = true;

		if(need_help)
		{
			std::cout << make_usage_string_(basename_(argv[0]), hidden, desc, p) << std::endl;
			return false;
		}
		return true;
	}

	bool parse_file(
			const std::string& filename,
			std::ifstream& fin,
			std::map<int, int>& sequences,
			std::vector<std::vector<Event>>& event_sequences,
			std::vector<std::vector<std::vector<DomainElement>>> &sequence_events,
			unsigned int& vector_size
	) {
		std::string extension(filename);
		extension = extension.substr(extension.size() - 4, 4);

		try {
            LineParser(fin, sequences, events, code_events, sequence_events, vector_size);
			fin.close();
		}
		catch(...) {
			std::cerr << "Error during " << filename << " parsing." << std::endl;
			return false;
		}

		return true;
	}

	bool parse_files(po::variables_map& vm) {
		std::map<int, int> sequences;
		std::map<int, int> sequences2;

		std::vector<std::shared_ptr<TC>> base_pct;
		std::vector<std::shared_ptr<TC>> base_itv;

		std::vector<std::vector<Event>> event_sequences;
		std::vector<std::vector<std::vector<DomainElement>>> sequence_events;
		std::vector<std::vector<std::vector<DomainElement>>> sequence_events2;

		std::vector<std::vector<std::vector<Event>>> trace;
		std::vector<std::vector<std::vector<Event>>> trace2;

		if (vectorSize) {
            DomainElementFactory::VECTOR_SIZE = vectorSize;
		}

		if (verbose) std::cerr << "[INFO] Positive file parsing" << std::endl;
		if (!parse_file(fname, positives, sequences, event_sequences, sequence_events, vectorSize)) return false;

		if (disc) {
			if (verbose) std::cerr << "[INFO] Negative file parsing" << std::endl;
			if (!parse_file(gname, negatives, sequences2, event_sequences, sequence_events2, vectorSize)) return false;
		}

		for (auto k = (unsigned) sequence_events.size(); k < code_events.size(); k++)
			sequence_events.emplace_back(sequences.size(), std::vector<DomainElement>());

		abstractions = std::vector<int>(code_events.size(), -1);

		Abstraction::generatePathways<Event>((int) events.size(), trace, sequence_events);
		if (disc) {
			Abstraction::generatePathways<Event>((int) events.size(), trace2, sequence_events2);
			for (auto k = (unsigned) trace.size(); k < trace2.size(); k++)
				trace.emplace_back(trace[0].size(), std::vector<Event>());
		}
		nb_se = (unsigned) abstractions.size();

		inv_events = std::vector<std::string>(events.size());
		for (auto e : events) inv_events[e.second] = e.first;

		setMinsup(trace[0].size());

		VerticalEventBase<Event>* base;
		base = new VerticalEventBase<Event>(trace,minsup);
		base->_cwm = DomainElementFactory::zero();
		b = base;
		b->classe = 0;

		if (disc) {
			base = new VerticalEventBase<Event>(trace2);
			base->_cwm = DomainElementFactory::zero();

			b2 = base;
			b2->classe = 1;
		}

		return true;
	}

	bool parse_vm(po::variables_map& vm) {
		if (vm.count("all_different")) dif = true;
		if (vm.count("close")) close = true;
		if (vm.count("not_calc_freq")) calc_freq = false;
		if (vm.count("json")) txt = false;
		if (vm.count("verbose")) verbose = true;
		if(vm.count("tid")) tid = true;

		positives.open(fname);
		if (!positives) {
			std::cerr << "Unable to open " << fname << "." << std::endl;
			return false;
		}

		if (!gname.empty()) {
			negatives.open(gname);
			if (!negatives) {
				std::cerr << "Unable to open " << gname << "." << std::endl;
				return false;
			}
			disc = true;
		}

		return true;
	}

	void setMinsup(unsigned long nb_seq) {
		if (fmin >= 1)
			minsup = (unsigned) ceil(fmin);
		else minsup = (unsigned) ceil(fmin * nb_seq);
	}

	std::string basename_(const std::string& p)
	{
#ifdef HAVE_BOOST_FILESYSTEM
		return boost::filesystem::path(p).stem().string();
#else
		size_t start = p.find_last_of('/');
		if(start == std::string::npos)
			start = 0;
		else
			++start;
		return p.substr(start);
#endif
	}

	std::vector<std::string> get_unlimited_positional_args_(const po::positional_options_description& p)
	{
		assert(p.max_total_count() == std::numeric_limits<unsigned>::max());
		std::vector<std::string> parts;
		const int MAX = 1000;
        const std::string &last = p.name_for_position(MAX);

		for(size_t i = 0; true; ++i)
		{
            const std::string &cur = p.name_for_position(static_cast<unsigned int>(i));
			if(cur == last)
			{
				parts.push_back(cur);
				parts.push_back('[' + cur + ']');
				parts.push_back("...");
				return parts;
			}
			parts.push_back(cur);
		}
		return parts;
	}

	std::string make_help_description(const std::string& program_name) {
		std::ostringstream oss;
		oss << "Description:" << std::endl;
		oss << program_name;
		oss << " implements an algorithm dedicated to extract discriminant chronicles with multi-dimensional input data." << std::endl;

		return oss.str();
	}

	std::string make_usage_string_(
			const std::string& program_name,
			const po::options_description& hidden,
			const po::options_description& desc,
			po::positional_options_description& p)
	{
		std::vector<std::string> parts;
		parts.push_back("Usage: ");
		parts.push_back(program_name);
		size_t N = p.max_total_count();
		if(N == std::numeric_limits<unsigned>::max())
		{
			std::vector<std::string> args = get_unlimited_positional_args_(p);
			parts.insert(parts.end(), args.begin(), args.end());
		}
		else
		{
			for(size_t i = 0; i < N; ++i)
			{
				parts.push_back(p.name_for_position((int) i));
			}
		}
		if(!desc.options().empty())
		{
			parts.emplace_back("[options]");
		}
		std::ostringstream oss;
		oss << make_help_description(program_name) << std::endl;
		std::copy(
				parts.begin(),
				parts.end(),
				std::ostream_iterator<std::string>(oss, " "));
		oss << std::endl << hidden;
		oss << std::endl << desc;
		return oss.str();
	}

private:
	std::ifstream positives;
	std::ifstream negatives;
	std::string fname;
	std::string gname;
	float fmin{};

public:

	unsigned minsup{};
	unsigned nb_se{};
	unsigned mincs;
	unsigned maxcs;
	unsigned vectorSize;

	float gmin;

	bool calc_freq;
	bool close;
	bool disc;
	bool dif;
	bool txt;
	bool verbose;
	bool tid;

	std::map<std::string, int> events;
	std::vector<std::string> inv_events;
	std::vector<std::string> code_events;
	std::vector<int> abstractions;
	Base* b;
	Base* b2;

	Options() : gname(""), mincs(0), maxcs(0), gmin(2), vectorSize(1),
                calc_freq(true), close(false), disc(false), dif(false), txt(true), verbose(false),
                tid(false), b(nullptr), b2(nullptr) {}
};

int main(const int argc, char* argv[])
{
	Options options;
	if(!options.parse(argc, argv))
		return EXIT_FAILURE;

	std::vector<std::shared_ptr<TC>> base_pct;
	std::vector<std::shared_ptr<TC>> base_itv;

	CDA algo;

	if (!options.disc) {
		std::vector<bool> conclusion(options.code_events.size(),false);
		if (options.verbose) std::cerr << "[INFO] Constraints base generation" << std::endl;
		algo.CCDC(options.b, conclusion, (int) options.code_events.size(),
				  (int) options.abstractions.size(), options.minsup,
				  0, options.dif, false, base_pct, base_itv);
		if (options.verbose) std::cerr << "[INFO] Temporal constraints number: " << algo.nb_tc << std::endl;
		signal (SIGINT,stopHandler);
	}

	if (options.verbose) std::cerr << "[INFO] Chronicles mining" << std::endl;

	if (options.disc) 
		algo.inv_events = options.inv_events;

	algo.txt = options.txt;
	algo.extract_with_tid = options.tid;

	std::vector<Chronicle> res;
	bool disc_exec = true;

	if (!options.disc)
		res = algo.run(*options.b, base_pct, base_itv, nullptr, options.minsup, (int) options.code_events.size(),
					   options.nb_se, options.verbose, 0, options.mincs, options.maxcs,
					   options.close, options.dif, false, options.calc_freq);
	else
		disc_exec = algo.run (*options.b, *options.b2, options.minsup, options.gmin,
				  (int) options.code_events.size(), options.mincs,
				  options.maxcs, options.verbose, options.close, false, false);

	delete options.b;
	if (options.disc) delete options.b2;
	else {
		for (const Chronicle &c : res) {
			if (options.txt)
				std::cout << c.txt(options.inv_events, nullptr) << std::endl;
			else
				std::cout << c.json(options.inv_events, nullptr) << std::endl;
		}
	}

	if (options.verbose && disc_exec) std::cerr << "[INFO] -- End --" << std::endl;
	
	return EXIT_SUCCESS;
}

