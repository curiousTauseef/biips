#define BOOST_TEST_MODULE BiipsTestCompiler
#include <boost/test/included/unit_test.hpp>

#include "config.hpp"
#include "BiipsVersion.hpp"
#include "storeUnregistered.hpp"
#include "TestIO.hpp"
#include "Console.hpp"
#include "kolmogorov.hpp"
#include "common/cholesky.hpp"
#include "iostream/ProgressBar.hpp"
#include "DumpReader.hpp"

#include <fstream>
#include <ctime>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/math/distributions/students_t.hpp>

namespace Biips
{
  const String BIIPSTEST_CONFIG_FILE_NAME = "biipstestcompiler.cfg";

  std::map<String, std::map<IndexRange, MultiArray> >
  extractStat(Console & console, StatTag tag,
              const Types<String>::Array & monitoredVar,
              const String & statName, Bool verbose, Bool smooth = false);

  Bool
  computeError(
      Scalar & error,
      const String & varName,
      const std::map<String, std::map<IndexRange, MultiArray> > & smcMeanValuesMap,
      const std::map<String, std::vector<MultiArray> > & benchValuesMap);
}

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

BOOST_AUTO_TEST_CASE( my_test )
{
  int argc = boost::unit_test::framework::master_test_suite().argc;
  char ** argv = boost::unit_test::framework::master_test_suite().argv;
  using namespace Biips;

#ifdef BIIPS_CATCH_TESTCOMPILER
  try
  {
#endif
  String model_file_name;
  Size exec_step;
  String do_smooth_str;
  Size check_mode;
  Size data_rng_seed;
  Size smc_rng_seed;
  vector<Size> n_particles;
  Scalar ess_threshold;
  String resample_type;
  Size n_smc;
  Scalar reject_level;
  String dot_file_name;
  String config_file_name;
  vector<String> mutations;
  Size verbosity;
  Size num_bins;
  String data_file_name;

  // Declare a group of options that will be
  // allowed only on command line
  po::options_description generic("Generic options");
  generic.add_options()("help-test,h", "produces help message.")(
      "version", "prints version string.")(
      "verbose,v", po::value<Size>(&verbosity)->default_value(1),
      "verbosity level.\n"
      "values:\n"
      " 0: \tnone.\n"
      " 1: \tminimal.\n"
      " 2: \thigh.")("interactive", "asks questions to the user.\n"
                     "applies when verbose>0.")(
      "num-bins", po::value<Size>(&num_bins)->default_value(40),
      "number of bins in the histograms.")(
      "step",
      po::value<Size>(&exec_step)->default_value(3),
      "execution step to be reached (if possible).\n"
      "values:\n"
      " 0: \tcompiles model, generates data and reads values from cfg file.\n"
      " 1: \t0 + runs SMC samplers and extract statistics.\n"
      " 2: \t1 + computes errors vs benchmarks.\n"
      " 3: \t2 + checks that errors are distributed according to reference SMC errors, when repeat-smc>1. checks that error is lesser than a 1-alpha quantile of the reference SMC errors, when repeat-smc=1.")(
      "smooth", po::value<String>(&do_smooth_str)->default_value("on"),
      "toggle backward smoothing step.\n"
      "values:\n"
      " on: \tenable backward smoothing step.\n"
      " off: \tdisable backward smoothing step.")(
      "check-mode", po::value<Size>(&check_mode)->default_value(2),
      "errors to be checked.\n"
      "values:\n"
      " 0: \tchecks normalizing-constant mean.\n"
      " 1: \t0 + checks filtering errors goodness of fit.\n"
      " 2: \t1 + checks smoothing errors goodness of fit.")
      ;

  // Declare a group of options that will be
  // allowed both on command line and in
  // config file
  po::options_description config("Configuration");
  config.add_options()("model-file", po::value<String>(&model_file_name),
                       "BUGS model file name.")(
      "data-file", po::value<String>(&data_file_name), "data file name.\n"
      "ASCII text containing data in the S-plus dump format.")(
      "data-rng-seed", po::value<Size>(&data_rng_seed),
      "data sampler rng seed. default=time().")(
      "particles",
      po::value<vector<Size> >(&n_particles)->default_value(
          vector<Size>(1, 1000)),
      "numbers of particles.")("smc-rng-seed", po::value<Size>(&smc_rng_seed),
                               "SMC sampler rng seed. default=time().\n"
                               "applies when repeat-smc=1.")(
      "mutations",
      po::value<vector<String> >(&mutations)->default_value(
          vector<String>(1, "optimal")),
      "type of exploration used in the mutation step of the SMC algorithm.\n"
      "values:\n"
      " optimal: \tuse optimal mutation, when possible.\n"
      " prior: \tuse prior mutation.\n"
      " <other>: \tpossible model-specific mutation.")(
      "resampling",
      po::value<String>(&resample_type)->default_value("stratified"),
      "resampling method.\n"
      "values:\n"
      " multinomial\n"
      " residual\n"
      " stratified\n"
      " systematic")("ess-threshold",
                     po::value<Scalar>(&ess_threshold)->default_value(0.5),
                     "ESS resampling threshold.")(
      "repeat-smc",
      po::value<Size>(&n_smc)->default_value(1),
      "number of independent SMC executions for each mutation and number of particles.")(
      "alpha", po::value<Scalar>(&reject_level)->default_value(0.01),
      "accepted level of rejection in checks.")
  //        ("plot-file", po::value<String>(&plot_file_name), "plots pdf file name.\n"
  //            "applies when repeat-smc=1.")
  ("dot-file", po::value<String>(&dot_file_name), "dot file name.\n"
   "The file will be created or overwritten.");

  // Hidden options, will be allowed both on command line and
  // in config file, but will not be show to the user.
  po::options_description hidden("Hidden options");
  hidden.add_options()(
      "cfg",
      po::value<String>(&config_file_name)->default_value(
          BIIPSTEST_CONFIG_FILE_NAME),
      "configuration file name.");

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  po::options_description config_file_options;
  config_file_options.add(config).add(hidden);

  po::options_description visible(
      "\nUsage: BiipsTestCompiler [<option>]... --model-file=<file_name> [<option>]...\n"
      "  runs model <file_name> with default model parameters, with multiple <option> parameters.\n"
      "       BiipsTestCompiler [<option>]... <config_file> [<option>]...\n"
      "  runs test configuration from file <config_file>, with multiple <option> parameters.\n"
      "\n"
      "Options, denoted here by <option>, are parsed from the general syntax:"
      "     --<long_key>[=<value>]\n"
      "  or -<short_key>[=<value>]\n"
      "where the bracketed text [...] is optional\n"
      "and the angle bracketed text <...> must be replaced by the corresponding value.\n"
      "\n"
      "Allowed options");
  visible.add(generic).add(config);

  po::positional_options_description pos_desc;
  pos_desc.add("cfg", 1);

  // map of parsed options from different sources
  vector<po::parsed_options> parsed_sources;
  vector<string> sources_names;

  // Command line options parsing
  po::variables_map vm;

  {
    parsed_sources.push_back(
        po::command_line_parser(argc, argv).options(cmdline_options).positional(
            pos_desc).run());
    sources_names.push_back("command-line");

    const po::parsed_options & parsed = parsed_sources.back();

#ifdef BIIPS_DEBUG
    if (parsed.options.size() > 0)
      cout << "[Command line options]" << endl;
    for (Size op = 0; op < parsed.options.size(); ++op)
    {
      cout << parsed.options[op].string_key;
      if (!parsed.options[op].value.empty())
        cout << " = ";
      for (Size v = 0; v < parsed.options[op].value.size(); ++v)
        cout << parsed.options[op].value[v] << " ";
      cout << endl;
    }
#endif

    store(parsed, vm);
    notify(vm);
  }

  // Program options pre-processing
  if (vm.count("help-test"))
  {
    cout << visible << "\n";
    return;
  }

  if (vm.count("version"))
  {
    cout << "BiipsTestCompiler, version " << BiipsVersion() << endl;
    return;
  }

  // Configuration file options parsing
  if (vm.count("cfg"))
  {
    ifstream ifs(config_file_name.c_str());
    if (ifs.fail())
      throw RuntimeError(String("Failed to open file ") + config_file_name);

    parsed_sources.push_back(
        po::parse_config_file(ifs, config_file_options, true));
    sources_names.push_back(config_file_name);

    const po::parsed_options & parsed = parsed_sources.back();

#ifdef BIIPS_DEBUG
    if (parsed.options.size() > 0)
      cout << "[Configuration options from file " << config_file_name << "]"
           << endl;
    for (Size op = 0; op < parsed.options.size(); ++op)
    {
      cout << parsed.options[op].string_key << " = ";
      for (Size v = 0; v < parsed.options[op].value.size(); ++v)
        cout << parsed.options[op].value[v] << endl;
    }
#endif

    store(parsed, vm);
    notify(vm);
  }

// Store unregistered
  vector<String> monitored_var;
  StoredDimMap dim_map_stored;
  StoredDataMap data_map_stored;
  Scalar log_norm_const_bench = 0.0;
  StoredDataMap bench_filter_map_stored;
  StoredDataMap bench_smooth_map_stored;
  StoredErrorsMap errors_filter_ref_map_stored;
  StoredErrorsMap errors_smooth_ref_map_stored;

  storeUnregistered(parsed_sources, sources_names, monitored_var,
                    dim_map_stored, data_map_stored, log_norm_const_bench,
                    bench_filter_map_stored, bench_smooth_map_stored,
                    errors_filter_ref_map_stored, errors_smooth_ref_map_stored);

  Bool interactive = vm.count("interactive");

  if (!vm.count("model-file"))
    boost::throw_exception(po::error("Missing model-file option."));

  Bool do_smooth;
  if (do_smooth_str == "on")
    do_smooth = true;
  else if (do_smooth_str == "off")
    do_smooth = false;
  else
    boost::throw_exception(
        po::validation_error(po::validation_error::invalid_bool_value,
                             do_smooth_str, "smooth"));

  // Read data
  // ---------------------------------
  map<String, MultiArray> data_map;

  if (vm.count("data-file")) // read from data dump file
  {
    if (verbosity > 0)
    {
      cout << PROMPT_STRING << "Reading data in:" << endl;
      cout << INDENT_STRING << "data-file = " << data_file_name << endl;
    }

    ifstream ifs_dump(data_file_name.c_str());
    if (ifs_dump.fail())
      throw RuntimeError(String("Failed to open file ") + data_file_name);

    DumpReader dump_reader(ifs_dump);
    while (ifs_dump.good())
    {
      if (!dump_reader.next())
        throw RuntimeError(String("Error parsing file: ") + data_file_name);

      DimArray::Ptr dim_ptr(
          new DimArray(dump_reader.dims().begin(), dump_reader.dims().end()));
      ValArray::Ptr val_ptr(new ValArray());
      if (dump_reader.is_int())
      {
        if (dump_reader.has_na())
        {
          val_ptr->resize(dump_reader.int_values().size());
          std::replace_copy(dump_reader.int_values().begin(),
                            dump_reader.int_values().end(), val_ptr->begin(),
                            Scalar(BIIPS_INTNA), BIIPS_REALNA);
        }
        else
        {
          val_ptr->assign(dump_reader.int_values().begin(),
                          dump_reader.int_values().end());
        }
      }
      else
      {
        val_ptr->assign(dump_reader.double_values().begin(),
                        dump_reader.double_values().end());
      }
      if (val_ptr->empty())
        throw RuntimeError(
            String("Error reading data: variable ") + dump_reader.name()
            + " has no values.");
      if (dim_ptr->empty())
        dim_ptr->push_back(val_ptr->size());
      else if (dim_ptr->Length() != val_ptr->size())
        throw RuntimeError(
            String("Error reading data: dimensions of variable ")
            + dump_reader.name() + " do not match the number of values.");

      data_map.insert(
          std::make_pair(dump_reader.name(), MultiArray(dim_ptr, val_ptr)));
    }

  }
  else // read from cfg file
  {
    if (verbosity > 0)
    {
      cout << PROMPT_STRING << "Reading data in:" << endl;
      cout << INDENT_STRING << "cfg = " << config_file_name << endl;
    }
    data_map = transformStoredDataMap(data_map_stored);

  }
#ifdef BIIPS_DEBUG
  cout << "Parsed data variables: ";
  for (map<String, MultiArray>::const_iterator it = data_map.begin();
      it != data_map.end(); ++it)
    cout << it->first << " ";
  cout << endl;
#endif

  if (verbosity > 0 && interactive)
    pressEnterToContinue();

  // Make a console
  // ------------------
  Console console(cout, cerr);

// Check model syntax
// ------------------
  if (!console.CheckModel(model_file_name, verbosity > 0))
    throw RuntimeError("Model syntax is incorrect.");

  if (verbosity > 0 && interactive)
    pressEnterToContinue();

  // Load Base module
  // ------------------
  if (!console.LoadBaseModule(verbosity > 0))
    throw RuntimeError("Failed to load Base module.");

  if (verbosity > 0 && interactive)
    pressEnterToContinue();

  // Compile model
  // ------------------

  if (!vm.count("data-rng-seed"))
    data_rng_seed = time(0);
  if (verbosity > 0)
    cout << INDENT_STRING << "data-rng-seed = " << data_rng_seed << endl;

  if (!console.Compile(data_map, true, data_rng_seed, verbosity))
    throw RuntimeError("Failed to compile model.");

  if (verbosity > 0 && interactive)
    pressEnterToContinue();

  // Write dot file
  //----------------------

  if (vm.count("dot-file"))
  {
    if (verbosity > 0)
    {
      cout << PROMPT_STRING << "Writing dot file in:" << endl;
      cout << INDENT_STRING << "dot-file = " << dot_file_name << endl;
    }

    std::ofstream ofs(dot_file_name.c_str());

    if (ofs.fail())
      throw RuntimeError(String("Failed to open file ") + dot_file_name);

    if (!console.PrintGraphviz(ofs))
      throw RuntimeError("Failed to print dot file.");

    if (verbosity > 0 && interactive)
      pressEnterToContinue();
  }

  if (exec_step < 1)
    return;

  // Monitor variables
  if (verbosity > 0)
    cout << PROMPT_STRING << "Setting user filter monitors" << endl;

  for (Size i = 0; i < monitored_var.size(); ++i)
  {
    const String & name = monitored_var[i];

    if (!console.SetFilterMonitor(name))
      throw RuntimeError(String("Failed to monitor variable ") + name);

    if (verbosity > 0)
      cout << INDENT_STRING << "monitoring variable " << name << endl;
  }

  if (verbosity > 0 && interactive && !monitored_var.empty())
    pressEnterToContinue();

  if (do_smooth)
  {
    if (verbosity > 0)
      cout << PROMPT_STRING
           << "Setting default filter monitors for backward smoothing step"
           << endl;

    if (!console.SetDefaultFilterMonitors())
      throw RuntimeError("Failed to set default filter monitors");

    if (verbosity > 0 && interactive && !monitored_var.empty())
      pressEnterToContinue();

    if (verbosity > 0)
      cout << PROMPT_STRING << "Setting user smoother monitors" << endl;

    for (Size i = 0; i < monitored_var.size(); ++i)
    {
      const String & name = monitored_var[i];

      if (!console.SetBackwardSmoothMonitor(name))
        throw RuntimeError(String("Failed to monitor variable ") + name);

      if (verbosity > 0)
        cout << INDENT_STRING << "monitoring variable " << name << endl;
    }

    if (verbosity > 0 && interactive && !monitored_var.empty())
      pressEnterToContinue();
  }

  // Run SMC samplers
  //----------------------
  for (Size i_n_part = 0; i_n_part < n_particles.size(); ++i_n_part)
  {
    Size n_part = n_particles[i_n_part];
    for (Size i_mut = 0; i_mut < mutations.size(); ++i_mut)
    {
      string mut = mutations[i_mut];

      if (verbosity > 0)
      {
        cout << PROMPT_STRING << "Running " << n_smc << " SMC algorithms";
        if (n_smc > 1 && do_smooth)
          cout << " with backward smoothing";
        cout << endl;
        cout << INDENT_STRING << "mutation = " << mut << endl;
        cout << INDENT_STRING << "particles = " << n_part << endl;
        cout << INDENT_STRING << "resampling = " << resample_type << endl;
        cout << INDENT_STRING << "ess-threshold = " << ess_threshold << endl;
      }

      if (verbosity > 0 && interactive && n_smc > 1)
        pressEnterToContinue();

      Types<ProgressBar>::Ptr p_show_progress;
      if (verbosity == 1 && n_smc > 1)
        p_show_progress = Types<ProgressBar>::Ptr(
            new ProgressBar(n_smc, cout, INDENT_STRING));

      vector<Scalar> errors_filter_new;
      vector<Scalar> errors_smooth_new;
      vector<Scalar> log_norm_const_smc;

      if (!console.BuildSampler(mut == "prior",
                                verbosity * (n_smc == 1 || verbosity > 1)))
        throw RuntimeError("Failed to build sampler.");

      if (verbosity > 0 && interactive && n_smc == 1)
        pressEnterToContinue();

      for (Size i_smc = 0; i_smc < n_smc; ++i_smc)
      {
        // Build sampler
        //----------------------
        if (n_smc > 1 || !vm.count("smc-rng-seed"))
          smc_rng_seed = time(0) + i_smc + 1;
        if (n_smc == 1 && verbosity > 0)
          cout << INDENT_STRING << "smc-rng-seed = " << smc_rng_seed << endl;

        if (verbosity > 0 && interactive && n_smc == 1)
          pressEnterToContinue();

        // Run sampler
        //----------------------
        Bool verbose_run_smc = verbosity > 1 || (verbosity > 0 && n_smc == 1);
        if (!console.RunForwardSampler(n_part, smc_rng_seed, resample_type,
                                       ess_threshold, verbose_run_smc,
                                       verbose_run_smc))
          throw RuntimeError("Failed to run SMC sampler.");

        Scalar log_norm_const;
        if (!console.GetLogNormConst(log_norm_const))
          throw RuntimeError("Failed to get log normalizing constant.");

        if (verbosity == 1 && n_smc > 1 && !do_smooth)
          ++(*p_show_progress);

        if (verbosity > 0 && n_smc == 1)
          cout << INDENT_STRING << "log-normalizing constant = "
               << log_norm_const << endl;

        log_norm_const_smc.push_back(log_norm_const);

        if (verbosity > 0 && interactive && n_smc == 1)
          pressEnterToContinue();

        // Extract filter mean of monitored values
        //----------------------------------------
        std::map<String, std::map<IndexRange, MultiArray> > filter_mean_map =
            extractStat(console, MEAN, monitored_var, "mean",
                        (verbosity > 0 && n_smc == 1), false);

        if (exec_step < 2)
          continue;

        // Compute filter mean error of monitored values
        //----------------------------------------------
        Scalar error_filter = 0.0;

        for (Size i = 0; i < monitored_var.size(); ++i)
        {
          const String & name = monitored_var[i];

          if (verbosity > 0 && n_smc == 1)
            cout << PROMPT_STRING << "Computing filtering error of variable "
                 << name << endl;

          if (!computeError(error_filter, name, filter_mean_map,
                            bench_filter_map_stored))
            throw RuntimeError(
                String("Failed to compute filtering error of variable ")
                + name);
        }

        error_filter *= n_part;

        if (verbosity > 0 && n_smc == 1)
          cout << INDENT_STRING << "filtering error = " << error_filter << endl;

        errors_filter_new.push_back(error_filter);

        if (verbosity > 0 && interactive && n_smc == 1)
          pressEnterToContinue();

        // Run backward smoother
        //-------------------------------------------------
        if (do_smooth)
        {
          Bool verbose_run_back = verbosity > 1
                                  || (verbosity > 0 && n_smc == 1);
          if (!console.RunBackwardSmoother(verbose_run_back, verbose_run_back))
            throw RuntimeError("Failed to run backward smoother.");

          if (verbosity == 1 && n_smc > 1)
            ++(*p_show_progress);

          if (verbosity > 0 && interactive && n_smc == 1)
            pressEnterToContinue();

          // Extract smooth mean of monitored values
          //----------------------------------------
          std::map<String, std::map<IndexRange, MultiArray> > smooth_mean_map =
              extractStat(console, MEAN, monitored_var, "mean",
                          (verbosity > 0 && n_smc == 1), true);

          if (exec_step < 2)
            continue;

          // Compute smooth mean error of monitored values
          //----------------------------------------------
          Scalar error_smooth = 0.0;

          for (Size i = 0; i < monitored_var.size(); ++i)
          {
            const String & name = monitored_var[i];

            if (verbosity > 0 && n_smc == 1)
              cout << PROMPT_STRING << "Computing smoothing error of variable "
                   << name << endl;

            if (!computeError(error_smooth, name, smooth_mean_map,
                              bench_smooth_map_stored))
              throw RuntimeError(
                  String("Failed to compute smoothing error of variable ")
                  + name);
          }

          error_smooth *= n_part;

          if (verbosity > 0 && n_smc == 1)
            cout << INDENT_STRING << "smoothing error = " << error_smooth
                 << endl;

          errors_smooth_new.push_back(error_smooth);

          if (verbosity > 0 && interactive && n_smc == 1)
            pressEnterToContinue();
        }
      }

      if (exec_step < 3)
        continue;

      // Checks
      //-------------------------------

      // Check normalizing constant mean;
      if (!log_norm_const_bench)
      {
        cerr << "Warning: no log-normalizing constant reference." << endl;
        cerr << "         missing 'log-norm-const' option in section [bench]."
             << endl;
      }
      else if (n_smc == 1)
      {
        cerr
            << "Warning: can not check normalizing constant mean with repeat-smc = 1."
            << endl;
      }
      else
      {
        cout << PROMPT_STRING
             << "Checking normalizing constant mean with reject level alpha = "
             << reject_level << endl;
        cout << INDENT_STRING << "expected log-norm-const mean = "
             << log_norm_const_bench << endl;

        using namespace boost::accumulators;
        typedef accumulator_set<long double, features<tag::mean, tag::variance> > acc_ref_type;

        acc_ref_type norm_const_smc_acc;

        for (Size i = 0; i < log_norm_const_smc.size(); ++i)
          norm_const_smc_acc(expl(log_norm_const_smc[i]));

        cout << INDENT_STRING << "SMC sample log-norm-const mean = "
        << log(mean(norm_const_smc_acc)) << endl;

        Scalar student_stat = (mean(norm_const_smc_acc)
                               - exp(log_norm_const_bench))
                              / sqrt(variance(norm_const_smc_acc))
                              * sqrt(Scalar(n_smc));
        boost::math::students_t student_dist(n_smc - 1);

        Scalar t_p_value = 2
                           * cdf(complement(student_dist, fabs(student_stat)));

        cout << INDENT_STRING << "Student t-test: t = " << student_stat
             << ", p-value = " << t_p_value << endl;

        BOOST_CHECK_GT(t_p_value, reject_level);

        if (verbosity > 0 && interactive)
          pressEnterToContinue();
      }

      // Check errors
      //-------------

      if (check_mode < 1)
        continue;

      Bool check_filter = check_mode >= 1;
      Bool check_smooth = check_mode >= 2 && do_smooth;

      if (check_filter)
      {
        if (!errors_filter_ref_map_stored.count(mut)
            || !errors_filter_ref_map_stored[mut].count(n_part))
        {
          cerr << "Warning: no filtering reference errors." << endl;
          cerr << "         missing " << mut << "." << n_part
               << " option in section [bench.filter]." << endl;
          check_filter = false;
        }
        else if (errors_filter_ref_map_stored[mut][n_part].empty())
        {
          cerr << "Warning: no filtering reference errors." << endl;
          cerr << "         option " << mut << "." << n_part
               << "  in section [bench.filter] has no value." << endl;
          check_filter = false;
        }
      }

      if (check_smooth)
      {
        if (!errors_smooth_ref_map_stored.count(mut)
            || !errors_smooth_ref_map_stored[mut].count(n_part))
        {
          cerr << "Warning: no smoothing reference errors." << endl;
          cerr << "         missing " << mut << "." << n_part
               << " option in section [bench.smooth]." << endl;
          check_smooth = false;
        }
        else if (errors_smooth_ref_map_stored[mut][n_part].empty())
        {
          cerr << "Warning: no smoothing reference errors." << endl;
          cerr << "         option " << mut << "." << n_part
               << "  in section [bench.smooth] has no value." << endl;
          check_smooth = false;
        }
      }

      if (n_smc == 1)
      {
        // compute 1-alpha quantile of reference errors distribution
        Scalar error_filter_threshold;
        Scalar error_smooth_threshold;

        if (verbosity > 0)
        {
          cout << PROMPT_STRING
               << "Checking errors < 1-alpha quantile of reference errors"
               << endl;
          cout << INDENT_STRING << "alpha = " << reject_level << endl;
        }

        using namespace boost::accumulators;
        typedef accumulator_set<Scalar, features<tag::p_square_quantile> > acc_ref_type;

        if (check_filter)
        {
          acc_ref_type errors_filter_ref_acc;

          errors_filter_ref_acc = acc_ref_type(
              quantile_probability = 1 - reject_level);
          for (Size i = 0; i < errors_filter_ref_map_stored[mut][n_part].size();
              ++i)
            errors_filter_ref_acc(errors_filter_ref_map_stored[mut][n_part][i]);
          error_filter_threshold = p_square_quantile(errors_filter_ref_acc);

          if (verbosity > 0)
            cout << INDENT_STRING << "filtering errors quantile = "
                 << error_filter_threshold << endl;

          BOOST_CHECK_LT(errors_filter_new.front(), error_filter_threshold);
        }

        if (check_smooth)
        {
          acc_ref_type errors_smooth_ref_acc;

          errors_smooth_ref_acc = acc_ref_type(
              quantile_probability = 1 - reject_level);
          for (Size i = 0; i < errors_smooth_ref_map_stored[mut][n_part].size();
              ++i)
            errors_smooth_ref_acc(errors_smooth_ref_map_stored[mut][n_part][i]);
          error_smooth_threshold = p_square_quantile(errors_smooth_ref_acc);

          if (verbosity > 0)
            cout << INDENT_STRING << "smoothing errors quantile = "
                 << error_smooth_threshold << endl;

          BOOST_CHECK_LT(errors_smooth_new.front(), error_smooth_threshold);
        }

        if (verbosity > 0 && interactive)
          pressEnterToContinue();
      }

      else if (n_smc > 1)
      {
        if (check_filter && !errors_filter_new.empty())
        {
          if (verbosity > 0)
            cout
                << PROMPT_STRING
                << "Checking filtering errors fitness with reject level alpha = "
                << reject_level << endl;

          Scalar ks_filter_stat = ksTwoSamplesStat(
              errors_filter_new.begin(), errors_filter_new.end(),
              errors_filter_ref_map_stored[mut][n_part].begin(),
              errors_filter_ref_map_stored[mut][n_part].end());

          Size n_err = errors_filter_new.size();
          Size n_err_ref = errors_filter_ref_map_stored[mut][n_part].size();
          Scalar ks_filter_prob = ksProb(
              sqrt(n_err * n_err_ref / (n_err + n_err_ref)) * ks_filter_stat);

          if (verbosity > 0)
            cout << INDENT_STRING << "K-S test: D = " << ks_filter_stat
                 << ", p-value = " << ks_filter_prob << endl;

          BOOST_CHECK_GT(ks_filter_prob, reject_level);

          if (verbosity > 0 && interactive)
            pressEnterToContinue();
        }

        if (check_smooth && !errors_smooth_new.empty())
        {
          if (verbosity > 0)
            cout
                << PROMPT_STRING
                << "Checking smoothing errors fitness with reject level alpha = "
                << reject_level << endl;

          Scalar ks_smooth_stat = ksTwoSamplesStat(
              errors_smooth_new.begin(), errors_smooth_new.end(),
              errors_smooth_ref_map_stored[mut][n_part].begin(),
              errors_smooth_ref_map_stored[mut][n_part].end());

          Size n_err = errors_smooth_new.size();
          Size n_err_ref = errors_smooth_ref_map_stored[mut][n_part].size();
          Scalar ks_smooth_prob = ksProb(
              sqrt(n_err * n_err_ref / (n_err + n_err_ref)) * ks_smooth_stat);

          if (verbosity > 0)
            cout << INDENT_STRING << "K-S test: D = " << ks_smooth_stat
                 << ", p-value = " << ks_smooth_prob << endl;

          BOOST_CHECK_GT(ks_smooth_prob, reject_level);
        }

        if (verbosity > 0 && interactive)
          pressEnterToContinue();
      }
    }
  }

#ifdef BIIPS_CATCH_TESTCOMPILER
}
BIIPS_CATCH_ERRORS
catch (po::error & err)
{
  cerr << "ERROR: " << err.what() << endl;
}
#endif
}

namespace Biips
{

  std::map<String, std::map<IndexRange, MultiArray> > extractStat(
      Console & console, StatTag tag, const Types<String>::Array & monitoredVar,
      const String & statName, Bool verbose, Bool smooth)
  {
    std::map<String, std::map<IndexRange, MultiArray> > stat_map;

    for (Size i = 0; i < monitoredVar.size(); ++i)
    {
      const String & name = monitoredVar[i];

      if (verbose)
      {
        if (!smooth)
          cout << PROMPT_STRING << "Extracting filtering " << statName
               << " of variable " << name << endl;
        else
          cout << PROMPT_STRING << "Extracting smoothing " << statName
               << " of variable " << name << endl;
      }

      if (!smooth)
      {
        if (!console.ExtractFilterStat(name, tag, stat_map[name]))
          throw RuntimeError(
              String("Failed to extract filtering stat of variable ") + name);
      }
      else
      {
        if (!console.ExtractBackwardSmoothStat(name, tag, stat_map[name]))
          throw RuntimeError(
              String("Failed to extract smoothing stat of variable ") + name);
      }
    }

    return stat_map;
  }

  Bool computeError(
      Scalar & error,
      const String & varName,
      const std::map<String, std::map<IndexRange, MultiArray> > & smcMeanValuesMap,
      const std::map<String, std::vector<MultiArray> > & benchValuesMap)
  {
    if (!(smcMeanValuesMap.count(varName) && benchValuesMap.count(varName)
          && benchValuesMap.count("var." + varName)))
      return false;

    const std::map<IndexRange, MultiArray> & smc_values_map = smcMeanValuesMap
        .find(varName)->second;
    const std::vector<MultiArray> & bench_values = benchValuesMap.find(varName)
        ->second;
    const std::vector<MultiArray> & bench_var_values = benchValuesMap.find(
        "var." + varName)->second;

    if (smc_values_map.size() != bench_values.size())
      return false;
    if (smc_values_map.size() != bench_var_values.size())
      return false;

    std::map<IndexRange, MultiArray>::const_iterator it_smc_values =
        smc_values_map.begin();
    for (Size k = 0; it_smc_values != smc_values_map.end();
        ++k, ++it_smc_values)
    {
      if (!(it_smc_values->second.IsVector() && bench_values[k].IsVector()))
        return false;

      Size dim = it_smc_values->second.Length();

      if (dim != bench_values[k].Length())
        return false;

      if (dim == 1)
      {
        if (bench_var_values[k].Length() != 1)
          return false;
      }
      else
      {
        if (!bench_var_values[k].IsSquared())
          return false;
        else if (dim != bench_var_values[k].Dim()[0])
          return false;
      }

      Vector diff_vec(
          dim, it_smc_values->second.Values() - bench_values[k].Values());
      Matrix var_chol(dim, dim, bench_var_values[k].Values());
      if (!ublas::cholesky_factorize(var_chol))
        throw RuntimeError(
            "computeError: matrix is not positive-semidefinite.");
      ublas::inplace_solve(var_chol, diff_vec, ublas::lower_tag());
      error += ublas::inner_prod(diff_vec, diff_vec);
    }

    return true;
  }
}
