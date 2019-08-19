#include "../../core/core_headers.h"
#include <wx/xml/xml.h>

class
WarpToCistemApp : public MyApp
{

	public:

	bool DoCalculation();
	MovieAsset LoadMovieFromWarp(wxXmlDocument warp_doc, wxString warp_folder, wxString movie_filename, unsigned long count, float wanted_binned_pixel_size);
	ImageAsset LoadImageFromWarp(wxString image_filename, unsigned long parent_asset_id, double parent_voltage, double parent_cs, bool parent_are_white);
	CTF LoadCTFFromWarp(wxXmlDocument warp_doc, float pixel_size, float voltage, float spherical_aberration, wxString wanted_avrot_filename);
	void DoInteractiveUserInput();

	wxString warp_directory;
	wxString cistem_parent_directory;
	wxString project_name;
	float wanted_binned_pixel_size;
	bool do_import_images;
	bool do_import_ctf_results;
//	bool do_import_particle_coordinates;
	Project new_project;
	private:
};


IMPLEMENT_APP(WarpToCistemApp)

void WarpToCistemApp::DoInteractiveUserInput()
{
	wxString warp_directory = "";
	wxString cistem_parent_directory = "";
	wxString project_name = "";
	float wanted_binned_pixel_size = 1.0;
	bool do_import_images = false;
	bool do_import_ctf_results = false;
//	bool do_import_particle_coordinates = true;
	UserInput *my_input = new UserInput("Warp to Cistem", 1.0);

	warp_directory = my_input->GetFilenameFromUser("Input Warp Directory", "The folder in which Warp processed movies", "./Data", false);
	cistem_parent_directory = my_input->GetFilenameFromUser("Cistem Project Parent Directory", "The parent directory for the new cistem project", "~/", false);
	project_name = my_input -> GetFilenameFromUser("Project Name", "Name for new cisTEM2 project", "New_Project", false);
	wanted_binned_pixel_size = my_input -> GetFloatFromUser("Binned Pixel Size", "Pixel size to resample movies to after import.", "1.0", 0.0);
	do_import_images = my_input -> GetYesNoFromUser("Import Images?", "Should we import aligned averaged images from WARP (using a different motion correction system than cisTEM)?", "Yes");
	if (do_import_images) {
		do_import_ctf_results = my_input -> GetYesNoFromUser("Import CTF Estimates?", "Should we import results of CTF estimation from Warp?", "Yes");
	} else do_import_ctf_results=false;
		//	do_import_particle_coordinates = my_input -> GetYesNoFromUser("Import Particle Coordinates?", "Should we import coordinates of particles picked by WARP (using BoxNet)?", "Yes");
	delete my_input;

	my_current_job.ManualSetArguments("tttbfb",	warp_directory.ToUTF8().data(), cistem_parent_directory.ToUTF8().data(), project_name.ToUTF8().data(), do_import_images, wanted_binned_pixel_size, do_import_ctf_results);

}

MovieAsset WarpToCistemApp::LoadMovieFromWarp(wxXmlDocument warp_doc, wxString warp_folder, wxString movie_filename, unsigned long count, float wanted_binned_pixel_size)
{
	MovieAsset new_asset = MovieAsset();
	new_asset.filename = movie_filename;
	new_asset.asset_name = new_asset.filename.GetName();
	new_asset.asset_id = count+1;
	new_asset.dark_filename = "";
	new_asset.output_binning_factor = 1.0;
	/* TODO: Replace this with logic to handle mag distortion info from Warp */
	new_asset.correct_mag_distortion = false;
	new_asset.mag_distortion_angle = 0.0;
	new_asset.mag_distortion_major_scale = 1.0;
	new_asset.mag_distortion_minor_scale = 1.0;
	wxString dimension_string = "";
	double pixel_size = 1.0;
	double cs = 2.7;
	double voltage=300;
	double dose_rate = 1.0;
	wxXmlNode *child_1 = warp_doc.GetRoot()->GetChildren();
	while (child_1) {
		if ((child_1)->GetName() == "OptionsCTF") {
			wxXmlNode *child_2 = child_1->GetChildren();
			while (child_2) {
				if (child_2->GetAttribute("Name") == "PixelSizeX") {
					wxString str_pixel_size = child_2->GetAttribute("Value");
					if(!str_pixel_size.ToDouble(&pixel_size)) {SendErrorAndCrash("Couldn't convert Pixel Size to a double");}
					new_asset.pixel_size = pixel_size;
					double binning_factor = wanted_binned_pixel_size/pixel_size;
					if (binning_factor >= 1.0) {
						new_asset.output_binning_factor = binning_factor;
					}
				}
				if (child_2->GetAttribute("Name") == "GainPath") {
					wxFileName gain_filename = wxFileName(child_2->GetAttribute("Value"), wxPATH_WIN);
					wxString adjusted_filename = warp_folder + gain_filename.GetFullName(); // This requires that the gain filename be in the warp folder! todo locate gain file more flexibly... user selected?
					new_asset.gain_filename = adjusted_filename;
				}
				if (child_2->GetAttribute("Name") == "Cs") {
					wxString str_cs = child_2->GetAttribute("Value");
					if(!str_cs.ToDouble(&cs)) {SendErrorAndCrash("Couldn't convert Spherical Aberration to a double");}
					new_asset.spherical_aberration = cs;
				}
				if (child_2->GetAttribute("Name") == "Voltage") {
					wxString str_voltage = child_2->GetAttribute("Value");
					if(!str_voltage.ToDouble(&voltage)) {SendErrorAndCrash("Couldn't convert Voltage to a double");}
					new_asset.microscope_voltage = voltage;
				}
				if (child_2->GetAttribute("Name") == "Dimensions") {
					dimension_string = child_2->GetAttribute("Value");
				}
				child_2 = child_2->GetNext();
			}
		}
		else if ((child_1)->GetName() == "OptionsMovieExport"){
			wxXmlNode *child_2 = child_1->GetChildren();
			while (child_2) {
				if (child_2->GetAttribute("Name") == "DosePerAngstromFrame") {
					wxString str_dose_rate = child_2->GetAttribute("Value");
					if(!str_dose_rate.ToDouble(&dose_rate)) {SendErrorAndCrash("Couldn't convert Dose Rate to a double");}
					new_asset.dose_per_frame = dose_rate;
				}
				child_2 = child_2->GetNext();
			}
		}
		child_1 = child_1->GetNext();
	}
	double x_size_angstroms = 0.0;
	double y_size_angstroms = 0.0;

	wxStringTokenizer tokenizer(dimension_string, ",");
	wxString str_x_size = tokenizer.GetNextToken();
	wxString str_y_size = tokenizer.GetNextToken();
	wxString str_number_of_frames = tokenizer.GetNextToken();
	if(!str_x_size.ToDouble(&x_size_angstroms)) {SendErrorAndCrash("Couldn't convert x size to a double");}
	int x_size = myroundint(x_size_angstroms/pixel_size);
	if(!str_y_size.ToDouble(&y_size_angstroms)) {SendErrorAndCrash("Couldn't convert y size to a double");}
	int y_size = myroundint(y_size_angstroms/pixel_size);
	int number_of_frames = wxAtoi(str_number_of_frames);
	new_asset.x_size = x_size;
	new_asset.y_size = y_size;
	new_asset.number_of_frames = number_of_frames;
	new_asset.total_dose = number_of_frames * new_asset.dose_per_frame;
	new_asset.protein_is_white = false;
	new_asset.is_valid = true;
	return new_asset;
}

ImageAsset WarpToCistemApp::LoadImageFromWarp(wxString image_filename, unsigned long parent_asset_id, double parent_voltage, double parent_cs, bool parent_is_white){
	ImageAsset new_asset = ImageAsset();
	new_asset.filename = image_filename;
	new_asset.asset_name = new_asset.filename.GetName();
	new_asset.parent_id = parent_asset_id;
	new_asset.asset_id = parent_asset_id;
	new_asset.alignment_id = parent_asset_id;
	new_asset.microscope_voltage = parent_voltage;
	new_asset.spherical_aberration = parent_cs;
	new_asset.protein_is_white = parent_is_white;
	ImageFile img_file(image_filename.ToStdString(), false);
	new_asset.x_size = img_file.ReturnXSize();
	new_asset.y_size = img_file.ReturnYSize();
	new_asset.pixel_size = img_file.ReturnPixelSize();
	new_asset.is_valid = true;
	return new_asset;
}

CTF WarpToCistemApp::LoadCTFFromWarp(wxXmlDocument warp_doc, float pixel_size, float voltage, float spherical_aberration, wxString wanted_avrot_filename) {
	double defocus = 0.0;
	wxString str_defocus;
	double defocus_delta = 0.0;
	wxString str_defocus_delta;
	double defocus_angle = 0.0;
	wxString str_defocus_angle;
	double defocus_1 = 0.0;
	double defocus_2 = 0.0;
//	double defocus_max = 2.0;
//	wxString str_defocus_max;
//	double defocus_min = 0.0;
//	wxString str_defocus_min;
	double amplitude_contrast = 0.07;
	wxString str_amplitude_contrast;
	double minimum_range = .1;
	double minimum_frequency = .1;
	wxString str_minimum_range;
	double maximum_range = 0.6;
	double maximum_frequency = 0.6;
	wxString str_maximum_range;

	double phase_shift = 0.0;
	wxString str_phase_shift;
	double resolution_estimate = 0.0;
	wxString str_resolution_estimate;

	str_resolution_estimate = warp_doc.GetRoot()->GetAttribute("CTFResolutionEstimate");
	if(!str_resolution_estimate.ToDouble(&resolution_estimate)) {SendErrorAndCrash("Couldn't convert Voltage to a double");}
	wxXmlNode *child_1 = warp_doc.GetRoot()->GetChildren();
	while (child_1) {
		// TODO - parse `PS1D` eventually... but not yet.
		if ((child_1)->GetName() == "OptionsCTF") {
			wxXmlNode *child_2 = child_1->GetChildren();
			while (child_2) {
				if (child_2->GetAttribute("Name") == "Amplitude") {
					str_amplitude_contrast = child_2->GetAttribute("Value");
					if(!str_amplitude_contrast.ToDouble(&amplitude_contrast)) {SendErrorAndCrash("Couldn't convert Amplitude to a double");}
				}
				if (child_2->GetAttribute("Name") == "RangeMax") {
					str_maximum_range = child_2->GetAttribute("Value");
					if(!str_maximum_range.ToDouble(&maximum_range)) {SendErrorAndCrash("Couldn't convert RangeMax to a double");}
					maximum_frequency = maximum_range / (2*pixel_size);

				}
				if (child_2->GetAttribute("Name") == "RangeMin") {
					str_minimum_range = child_2->GetAttribute("Value");
					if(!str_minimum_range.ToDouble(&minimum_range)) {SendErrorAndCrash("Couldn't convert RangeMin to a double");}
					minimum_frequency = minimum_range / (2*pixel_size);
				}
				child_2 = child_2->GetNext();
			}
		}
		else if ((child_1)->GetName() == "CTF"){
			wxXmlNode *child_2 = child_1->GetChildren();
			while (child_2) {
				if (child_2->GetAttribute("Name") == "DefocusAngle") {
					str_defocus_angle = child_2->GetAttribute("Value");
					if(!str_defocus_angle.ToDouble(&defocus_angle)) {SendErrorAndCrash("Couldn't convert DefocusAngle to a double");}
				}
				if (child_2->GetAttribute("Name") == "Defocus") {
					str_defocus = child_2->GetAttribute("Value");
					if(!str_defocus.ToDouble(&defocus)) {SendErrorAndCrash("Couldn't convert Defocus to a double");}
				}
				if (child_2->GetAttribute("Name") == "DefocusDelta") {
					str_defocus_delta = child_2->GetAttribute("Value");
					if(!str_defocus_delta.ToDouble(&defocus_delta)) {SendErrorAndCrash("Couldn't convert DefocusDelta to a double");}
				}
				if (child_2->GetAttribute("Name") == "PhaseShift") {
					str_phase_shift = child_2->GetAttribute("Value");
					if(!str_phase_shift.ToDouble(&phase_shift)) {SendErrorAndCrash("Couldn't convert RangeMin to a double");}
				}
				child_2 = child_2->GetNext();
			}
		}
		child_1 = child_1->GetNext();
	}

	defocus_1 = (defocus + defocus_delta/2.0)*10000; // convert to Å, warp uses um
	defocus_2 = (defocus - defocus_delta/2.0)*10000; // convert to Å, warp uses um
	if (defocus_angle >90) defocus_angle += -180; // convert from 0 to 180 to -90 to 90
	CTF return_ctf = CTF(voltage,
			spherical_aberration,
			(float) amplitude_contrast,
			(float) defocus_1,
			(float) defocus_2,
			(float) defocus_angle,
			(float) minimum_frequency,
			(float) maximum_frequency,
			-1.0f,
			pixel_size,
			(float) phase_shift,
			0.0f,
			0.0f,
			0.0f,
			0.0f); //A
	return_ctf.SetHighestFrequencyWithGoodFit(1.0/resolution_estimate);

	// Write out the avrot file here so we have it in the same scope as the xmldoc for when we want to actually scrape the plot.
	wxTextFile *avrot_file = new wxTextFile(wanted_avrot_filename);
	// Stub file - all 1 all the time
	avrot_file->AddLine(wxString("# This file is a stub that does not reflect the data from Warp"));
	for (long counter=0; counter<6; counter++) {
		avrot_file->AddLine(wxString("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	}
	avrot_file->Write();
	avrot_file->Close();

	return return_ctf;
}

bool WarpToCistemApp::DoCalculation()
{
	wxString warp_directory = my_current_job.arguments[0].ReturnStringArgument();
	wxString cistem_parent_directory = my_current_job.arguments[1].ReturnStringArgument();
	wxString project_name = my_current_job.arguments[2].ReturnStringArgument();
	bool do_import_images = my_current_job.arguments[3].ReturnBoolArgument();
	float wanted_binned_pixel_size = my_current_job.arguments[4].ReturnFloatArgument();
	bool do_import_ctf_results = my_current_job.arguments[5].ReturnBoolArgument();

	ProgressBar *my_progress;
	long counter;

	wxPrintf("\nGenerating New cisTEM Project...\n\n");

	if (warp_directory.EndsWith("/") == false) warp_directory += "/";
	if (cistem_parent_directory.EndsWith("/") == false) cistem_parent_directory += "/";
	wxString wanted_folder_name = cistem_parent_directory + project_name;
	if (wxFileName::Exists(wanted_folder_name))
	{
		SendErrorAndCrash("Database directory should not already exist, and does!\n");
	}
	else wxFileName::Mkdir(wanted_folder_name);
	wxFileName wanted_database_file = wanted_folder_name + "/" + project_name + ".db";


	Project new_project = Project();
	new_project.CreateNewProject(wanted_database_file, wanted_folder_name, project_name);
	wxPrintf(wanted_database_file.GetFullPath()+"\n");
	wxPrintf("\nSuccessfully made project database\n\n");

	wxPrintf("\nImporting files from Warp...\n\n");

	wxArrayString all_files;
	wxDir::GetAllFiles 	( warp_directory, &all_files, "*.mrc", wxDIR_FILES);
	wxDir::GetAllFiles 	( warp_directory, &all_files, "*.mrcs", wxDIR_FILES);
	wxDir::GetAllFiles 	( warp_directory, &all_files, "*.tif", wxDIR_FILES);
	all_files.Sort();
	wxXmlDocument doc;
	size_t number_of_files = all_files.GetCount();
	wxString xml_filename;
	wxString image_filename;
	MovieAssetList movie_list = MovieAssetList();
	MovieAsset new_movie_asset;
	ImageAssetList image_list = ImageAssetList();
	ImageAsset new_image_asset;
	std::vector<CTF> ctf_list;
	CTF new_ctf_asset;
	if (all_files.IsEmpty() == true) {SendErrorAndCrash("No movies were detected in the warp directory.");}
	my_progress = new ProgressBar(number_of_files);

	// Load filenames into memory first, then we'll do batch database inserts later.
	for (counter = 0; counter < all_files.GetCount(); counter++)
	{
		size_t split_point = all_files.Item(counter).find_last_of(".");
		xml_filename = all_files.Item(counter);
		xml_filename = xml_filename.Mid(0, split_point);
		xml_filename.Append(".xml");
		// Check if warp xml exists before trying to do any more inserts
		if (wxFileName::Exists(xml_filename) && doc.Load(xml_filename) && doc.IsOk())
		{
			new_movie_asset = LoadMovieFromWarp(doc, warp_directory, all_files.Item(counter), counter, wanted_binned_pixel_size);
			movie_list.AddAsset(&new_movie_asset);

			image_filename = warp_directory + "average/" + wxFileName(xml_filename).GetName() + ".mrc";
			if (do_import_images && wxFileName(image_filename).IsOk() == true && wxFileName(image_filename).FileExists() == true) {
				new_image_asset = LoadImageFromWarp(image_filename, new_movie_asset.asset_id, new_movie_asset.microscope_voltage, new_movie_asset.spherical_aberration, new_movie_asset.protein_is_white);
				// Need to do the ctf first to adjust the ctf_estimation_id if necessary
				if (do_import_ctf_results) {
					new_image_asset.ctf_estimation_id = new_image_asset.asset_id;
					wxString wanted_avrot_filename = wanted_folder_name + wxString::Format("/Assets/CTF/%s_CTF_0_avrot.txt", new_image_asset.asset_name);

					CTF new_ctf_asset = LoadCTFFromWarp(doc, new_image_asset.pixel_size, new_movie_asset.microscope_voltage, new_movie_asset.spherical_aberration, wanted_avrot_filename);
					ctf_list.push_back(new_ctf_asset);
				}
				image_list.AddAsset(&new_image_asset);
			} else if (do_import_images) wxPrintf("Couldn't find averaged image: %s\n", image_filename);

		}
		else wxPrintf("Couldn't find a warp xml output for movie " + all_files.Item(counter) + "\n");
		my_progress->Update(counter+1);
	}

	// DB write operations and moving/writing files.

	delete my_progress;
	wxPrintf("\nSuccessfully imported files\n\n");

	new_project.database.Begin();

	wxPrintf("\nWriting movies to database\n\n");

	my_progress = new ProgressBar(movie_list.number_of_assets);
	new_project.database.BeginMovieAssetInsert();
	for (counter = 0; counter < movie_list.number_of_assets; counter++){
		new_movie_asset = reinterpret_cast <MovieAsset *> (movie_list.assets)[counter];
		new_project.database.AddNextMovieAsset(new_movie_asset.asset_id, new_movie_asset.asset_name, new_movie_asset.filename.GetFullPath(), 1, new_movie_asset.x_size, new_movie_asset.y_size, new_movie_asset.number_of_frames, new_movie_asset.microscope_voltage, new_movie_asset.pixel_size, new_movie_asset.dose_per_frame, new_movie_asset.spherical_aberration,new_movie_asset.gain_filename,new_movie_asset.dark_filename, new_movie_asset.output_binning_factor, new_movie_asset.correct_mag_distortion, new_movie_asset.mag_distortion_angle, new_movie_asset.mag_distortion_major_scale, new_movie_asset.mag_distortion_minor_scale, new_movie_asset.protein_is_white);
		my_progress->Update(counter+1);
	}
	new_project.database.EndMovieAssetInsert();
	delete my_progress;

	wxPrintf("\nSuccessfully imported movies\n\n");

	// Currently takes ~0.4 seconds per image to do these operations, driven mostly by the scaled image writing.
	if (do_import_images) {
		wxPrintf("\nWriting motion-corrected images to database and writing scaled images and spectra.\n\n");
		wxDateTime now = wxDateTime::Now();
		my_progress = new ProgressBar(image_list.number_of_assets);
		new_project.database.BeginImageAssetInsert();
		for (counter = 0; counter < image_list.number_of_assets; counter++){
			new_image_asset = reinterpret_cast <ImageAsset *> (image_list.assets)[counter];
			new_project.database.AddNextImageAsset(new_image_asset.asset_id, new_image_asset.asset_name, new_image_asset.filename.GetFullPath(), new_image_asset.position_in_stack, new_image_asset.parent_id, new_image_asset.alignment_id, new_image_asset.ctf_estimation_id, new_image_asset.x_size, new_image_asset.y_size, new_image_asset.microscope_voltage, new_image_asset.pixel_size, new_image_asset.spherical_aberration, new_image_asset.protein_is_white);
			my_progress->Update(counter+1);
		}
		new_project.database.EndImageAssetInsert();
		delete my_progress;

		wxPrintf("\nDone with database insert of images\n\n");

		wxPrintf("\nPopulating Image Alignment Jobs in Database\n\n");
		my_progress = new ProgressBar(image_list.number_of_assets);
		new_project.database.BeginBatchInsert("MOVIE_ALIGNMENT_LIST", 22, "ALIGNMENT_ID", "DATETIME_OF_RUN", "ALIGNMENT_JOB_ID", "MOVIE_ASSET_ID", "OUTPUT_FILE", "VOLTAGE", "PIXEL_SIZE", "EXPOSURE_PER_FRAME", "PRE_EXPOSURE_AMOUNT", "MIN_SHIFT", "MAX_SHIFT", "SHOULD_DOSE_FILTER", "SHOULD_RESTORE_POWER", "TERMINATION_THRESHOLD", "MAX_ITERATIONS", "BFACTOR", "SHOULD_MASK_CENTRAL_CROSS", "HORIZONTAL_MASK", "VERTICAL_MASK", "SHOULD_INCLUDE_ALL_FRAMES_IN_SUM", "FIRST_FRAME_TO_SUM", "LAST_FRAME_TO_SUM" );
		for (counter = 0; counter < image_list.number_of_assets; counter++){
			new_image_asset = reinterpret_cast <ImageAsset *> (image_list.assets)[counter];
			new_project.database.AddToBatchInsert("iliitrrrrrriiriiiiiiii", new_image_asset.alignment_id,
																			(long int) now.GetAsDOS(),
																			1, //alignment_job_id - set to 1
																			new_image_asset.asset_id,
																			new_image_asset.filename.GetFullPath().ToStdString().c_str(),
																			new_image_asset.microscope_voltage,
																			new_image_asset.pixel_size,
																			0.0, // exposure per frame
																			0.0, //current_pre_exposure
																			0.0, // min shift
																			0.0, // max shift
																			1, // should dose filter
																			1, // should restore power
																			1.0, // termination threshold
																			20, // max iterations
																			1500, //b factor
																			1, //mask central cross
																			1, // horizontal mask
																			1, // vertical cross
																			1, // include all frames
																			1, //first frame
																			0); //last frames
			my_progress->Update(counter + 1);
		}
		new_project.database.EndBatchInsert();

		delete my_progress;

		my_progress = new ProgressBar(image_list.number_of_assets);
		wxString current_table_name;
		long frame_counter;
		for (counter = 0; counter < image_list.number_of_assets; counter++){
			new_image_asset = reinterpret_cast <ImageAsset *> (image_list.assets)[counter];
			current_table_name = wxString::Format("MOVIE_ALIGNMENT_PARAMETERS_%i", new_image_asset.alignment_id);
			new_project.database.CreateTable(current_table_name, "prr", "FRAME_NUMBER", "X_SHIFT", "Y_SHIFT");
			new_project.database.BeginBatchInsert(current_table_name, 3, "FRAME_NUMBER", "X_SHIFT", "Y_SHIFT");
			for (frame_counter = 0; frame_counter < 40; frame_counter++) // 5 is totally arbitrary
			{
				new_project.database.AddToBatchInsert("irr", frame_counter+1, 0.0, 0.0);
			}
			new_project.database.EndBatchInsert();
			my_progress->Update(counter+1);
		}

		if (do_import_ctf_results) {
			wxPrintf("\nInserting CTF results in database\n\n");
			wxDateTime now = wxDateTime::Now();
			my_progress = new ProgressBar(image_list.number_of_assets);
			new_project.database.BeginBatchInsert("ESTIMATED_CTF_PARAMETERS", 32,
												  "CTF_ESTIMATION_ID",
												  "CTF_ESTIMATION_JOB_ID",
												  "DATETIME_OF_RUN",
												  "IMAGE_ASSET_ID",
												  "ESTIMATED_ON_MOVIE_FRAMES",
												  "VOLTAGE",
												  "SPHERICAL_ABERRATION",
												  "PIXEL_SIZE",
												  "AMPLITUDE_CONTRAST",
												  "BOX_SIZE",
												  "MIN_RESOLUTION",
												  "MAX_RESOLUTION",
												  "MIN_DEFOCUS",
												  "MAX_DEFOCUS",
												  "DEFOCUS_STEP",
												  "RESTRAIN_ASTIGMATISM",
												  "TOLERATED_ASTIGMATISM",
												  "FIND_ADDITIONAL_PHASE_SHIFT",
												  "MIN_PHASE_SHIFT",
												  "MAX_PHASE_SHIFT",
												  "PHASE_SHIFT_STEP",
												  "DEFOCUS1",
												  "DEFOCUS2",
												  "DEFOCUS_ANGLE",
												  "ADDITIONAL_PHASE_SHIFT",
												  "SCORE",
												  "DETECTED_RING_RESOLUTION",
												  "DETECTED_ALIAS_RESOLUTION",
												  "OUTPUT_DIAGNOSTIC_FILE",
												  "NUMBER_OF_FRAMES_AVERAGED",
												  "LARGE_ASTIGMATISM_EXPECTED",
												  "ICINESS");
			for	(counter = 0; counter < image_list.number_of_assets; counter++) {
				new_image_asset = reinterpret_cast <ImageAsset *> (image_list.assets)[counter];
				CTF new_ctf = ctf_list.at(counter);
				wxString wanted_ctf_filename = wanted_folder_name + wxString::Format("/Assets/CTF/%s_CTF_0.mrc", new_image_asset.asset_name); // This logic is repeated before - its the only case of straight repeating code I've resorted to, but calculating it twice is easier than storing the data in an object somewhere for multiple iterations.
				new_project.database.AddToBatchInsert("iiliirrrrirrrrririrrrrrrrrrrtiir",
													new_image_asset.ctf_estimation_id, // Image asset join column reference
													1, // CTF Job id
													(long int) now.GetAsDOS(), // datetime
													new_image_asset.asset_id, // Image asset parent
													1, // Estimated on movie frames - I am not scraping this from Warp because it doesn't fit in the CTF object easily, and doesn't add enough to be worth adding to the object definition.
													new_image_asset.microscope_voltage, // I could get this from the wavelength of the CTF object, but it doesn't seem necessary.
													new_ctf.GetSphericalAberration() /10000000.0*new_image_asset.pixel_size, // convert back to mm
													new_image_asset.pixel_size,
													new_ctf.GetAmplitudeContrast(),
													512, // box size - I think this is the box size of the spectrum and am filling on that assumption.
													new_image_asset.pixel_size/new_ctf.GetLowestFrequencyForFitting(), // Invert the frequency
													new_image_asset.pixel_size/new_ctf.GetHighestFrequencyForFitting(), // Invert the frequency
													10000*0.0, // Min defocus - I am not scraping this from warp (same as ESTIMATED_ON_MOVIE_FRAMES) but these are the defaults as of 190815
													10000*2.0, // Max defocus - I am not scraping this from warp (same as above).
													-1, // Defocus Step is not reported by warp anywhere... this is negative for safety.
													0, // Restrain Astigmatism is not selectable by warp.
													-1, //Tolerated Astigmatism is not selectable by warp.
													0, // Not scraping find additional phase shift - See above, I don't want to add all this to CTF unless I have to.
													0.0, // Min Phase
													0.0, // Max Phase
													0.0, // Phase shift step
													new_ctf.GetDefocus1()*new_image_asset.pixel_size,
													new_ctf.GetDefocus2()*new_image_asset.pixel_size,
													new_ctf.GetAstigmatismAzimuth()*180/PI, // Convert back to degrees
													new_ctf.GetAdditionalPhaseShift(),
													-1.0, // Score
													1.0/new_ctf.GetHighestFrequencyWithGoodFit(), //Resolution of fit.
													0.0, // Alias Resolution
													wanted_ctf_filename.ToStdString().c_str(),
													-1, // Number of frames averaged
													0, // Large Astigmatism Expected is not used by warp
													-1.0); // Iciness is not calculated by warp

				my_progress->Update(counter+1);
			}
			delete my_progress;
			new_project.database.EndBatchInsert();
		}


		new_project.database.Commit();

		wxPrintf("\nDone with database operations\n\n");

		wxPrintf("\nPreparing scaled images and spectra\n\n");
		my_progress = new ProgressBar(image_list.number_of_assets);
		Image large_image;
		Image buffer_image;
		float average;
		float sigma;
		for (counter = 0; counter < image_list.number_of_assets; counter++)
		{
			new_image_asset = reinterpret_cast <ImageAsset *> (image_list.assets)[counter];
			//Spectrum
			wxString wanted_spectrum_filename = wanted_folder_name + wxString::Format("/Assets/Images/Spectra/%s.mrc", new_image_asset.asset_name); //, new_image_asset.asset_id, 0); _%i_%i
			wxString current_spectrum_filename = warp_directory + wxString::Format("powerspectrum/%s.mrc", new_image_asset.asset_name);

			// Warp spectra are halved and unthresholded, so this fixes that.
			large_image.QuickAndDirtyReadSlice(current_spectrum_filename.ToStdString(), 1); // reusing large image and buffer_image here
			buffer_image.Allocate(large_image.logical_x_dimension, large_image.logical_y_dimension * 2, 1);
			for (int output_address = 0; output_address < large_image.real_memory_allocated; output_address++)
			{
				buffer_image.real_values[output_address] = large_image.real_values[output_address];
			}
			int input_address = buffer_image.real_memory_allocated / 2;
			for (int address = large_image.real_memory_allocated - 1; address >= 0; address--)
			{
			   buffer_image.real_values[input_address] = large_image.real_values[address];
			   input_address++;
			}

			buffer_image.CosineRingMask(0, buffer_image.logical_x_dimension / 2, 5.0f);
			buffer_image.ForwardFFT();
			buffer_image.CosineMask(0, 0.05, true);
			buffer_image.BackwardFFT();
			buffer_image.ComputeAverageAndSigmaOfValuesInSpectrum(float(buffer_image.logical_x_dimension)*0.5,float(buffer_image.logical_x_dimension),average,sigma,12);
			buffer_image.DivideByConstant(sigma);
			buffer_image.SetMaximumValueOnCentralCross(average/sigma+10.0);

			buffer_image.ComputeAverageAndSigmaOfValuesInSpectrum(float(buffer_image.logical_x_dimension)*0.5,float(buffer_image.logical_x_dimension),average,sigma,12);
			buffer_image.SetMinimumAndMaximumValues(average - 15.0, average + 15.0);
			buffer_image.QuickAndDirtyWriteSlice(wanted_spectrum_filename.ToStdString(), 1);
			// Also write fake diagnostic ctf spectrum in this scope to reuse the work done above.
			if (do_import_ctf_results) {
				wxString wanted_ctf_filename = wanted_folder_name + wxString::Format("/Assets/CTF/%s_CTF_0.mrc", new_image_asset.asset_name);
				buffer_image.QuickAndDirtyWriteSlice(wanted_ctf_filename.ToStdString(), 1,true);
			}

			//Scaled Image - borrowed the logic heavily from Unblur
			wxString wanted_scaled_filename = wanted_folder_name + wxString::Format("/Assets/Images/Scaled/%s.mrc", new_image_asset.asset_name); //, new_image_asset.asset_id, 0); _%i_%i

			int largest_dimension =  std::max(new_image_asset.x_size, new_image_asset.y_size);
			float scale_factor = float(SCALED_IMAGE_SIZE) / float(largest_dimension);
			if (scale_factor > 1) scale_factor=1.0;
			large_image.QuickAndDirtyReadSlice(new_image_asset.filename.GetFullPath().ToStdString(), 1);
			large_image.ForwardFFT();
			buffer_image.Allocate(myroundint(new_image_asset.x_size*scale_factor), myroundint(new_image_asset.y_size*scale_factor), false);
			large_image.ClipInto(&buffer_image);
			buffer_image.BackwardFFT();
			buffer_image.QuickAndDirtyWriteSlice(wanted_scaled_filename.ToStdString(), 1,true);

			my_progress->Update(counter+1);
		}
		delete my_progress;
		wxPrintf("\nDone writing spectra and scaled images\n\n");
	}

	wxPrintf("\ncisTEM project ready to be loaded by GUI.\n");
	return true;
}

