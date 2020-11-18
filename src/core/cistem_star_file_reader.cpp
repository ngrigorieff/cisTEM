#include "core_headers.h"

cisTEMStarFileReader::cisTEMStarFileReader()
{
	Reset();
}

cisTEMStarFileReader::cisTEMStarFileReader(wxString wanted_filename, ArrayOfcisTEMParameterLines *alternate_cached_parameters_pointer, bool exclude_negative_film_numbers)
{
	Reset();

	if (alternate_cached_parameters_pointer == NULL)
	{
		cached_parameters = new ArrayOfcisTEMParameterLines;
		using_external_array = false;
	}
	else
	{
		cached_parameters = alternate_cached_parameters_pointer;
		using_external_array = true;
	}

	ReadFile(wanted_filename, NULL, alternate_cached_parameters_pointer, exclude_negative_film_numbers);
}

void cisTEMStarFileReader::Reset()
{
	filename = "";
	input_file = NULL;

	current_position_in_stack = 0;
	current_column = 0;

	cached_parameters = NULL;
	using_external_array = false;

	ResetColumnPositions();

	parameters_that_were_read.SetAllToFalse();
}


void cisTEMStarFileReader::ResetColumnPositions()
{
	position_in_stack_column = -1;
	image_is_active_column = -1;
	psi_column = -1;
	theta_column = -1;
	phi_column = -1;
	x_shift_column = -1;
	y_shift_column = -1;
	defocus_1_column = -1;
	defocus_2_column = -1;
	defocus_angle_column = -1;
	phase_shift_column = -1;
	occupancy_column = -1;
	logp_column = -1;
	sigma_column = -1;
	score_column = -1;
	score_change_column = -1;
	pixel_size_column = -1;
	microscope_voltage_kv_column = -1;
	microscope_spherical_aberration_mm_column = -1;
	amplitude_contrast_column = -1;
	beam_tilt_x_column = -1;
	beam_tilt_y_column = -1;
	image_shift_x_column = -1;
	image_shift_y_column = -1;
	best_2d_class_column = -1;
	beam_tilt_group_column = -1;
	particle_group_column = -1;
	assigned_subset_column = -1;
	pre_exposure_column = -1;
	total_exposure_column = -1;
	original_image_filename_column = -1;
	reference_3d_filename_column = -1;
	stack_filename_column = -1;
}



cisTEMStarFileReader::~cisTEMStarFileReader()
{
	Close();
}

void cisTEMStarFileReader::Open(wxString wanted_filename, ArrayOfcisTEMParameterLines *alternate_cached_parameters_pointer)
{
	Close();

	if (cached_parameters != NULL && using_external_array == false)
	{
		delete cached_parameters;
		cached_parameters = NULL;
	}
	filename = wanted_filename;

	input_file = new wxTextFile(wanted_filename);
	input_file->Open();

	//if (input_file_stream->IsOk() == false)
	if (input_file->IsOpened() == false)
	{
		MyPrintWithDetails("Error: Cannot open star file (%s) for read\n", wanted_filename);
		DEBUG_ABORT;
	}

	if (alternate_cached_parameters_pointer == NULL)
	{
		cached_parameters = new ArrayOfcisTEMParameterLines;
		using_external_array = false;
	}
	else
	{
		cached_parameters = alternate_cached_parameters_pointer;
		using_external_array = true;
	}
}

void cisTEMStarFileReader::Close()
{
	if (cached_parameters != NULL && using_external_array == false)
	{
		delete cached_parameters;
		cached_parameters = NULL;
	}
	if (input_file != NULL) delete input_file;

}

bool cisTEMStarFileReader::ExtractParametersFromLine(wxString &wanted_line, wxString *error_string, bool exclude_negative_film_numbers)
{
	/*! \brief Parse a line read from a star file with checks on numeric convertibility.
	 *
	 * 	Detailed:
	 *		Each potential variable has a default column i.d. of -1. If this is not changed, this method will not try to parse the variable,
	 *		but instead will set it's respective default.
	 */

	// extract info.

	wxArrayString all_tokens;
	wxStringTokenizer tokens(wanted_line);
	cisTEMParameterLine temp_parameters;

	double temp_double;
	long temp_long;

	while (tokens.HasMoreTokens() == true)
	{
		all_tokens.Add(tokens.GetNextToken());
	}

	// start with image is active, because sometimes we can just stop..

	// image is active

	if (image_is_active_column == -1) temp_parameters.image_is_active = 1.0;
	else
	{
		if (all_tokens[image_is_active_column].ToLong(&temp_long) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[image_is_active_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[image_is_active_column]);
			return false;
		}

		temp_parameters.image_is_active = int(temp_long);
	}

	if (temp_parameters.image_is_active < 0 && exclude_negative_film_numbers == true) return true;

	// position in stack

	if (position_in_stack_column == -1) temp_double = -1;
	else
	if (all_tokens[position_in_stack_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[position_in_stack_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[position_in_stack_column]);
		return false;
	}

	temp_parameters.position_in_stack = int(temp_double);


	// phi

	if (phi_column == -1) temp_double = 0.0;
	else
	if (all_tokens[phi_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[phi_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[phi_column]);
		return false;
	}

	temp_parameters.phi = float(temp_double);

	// theta

	if (theta_column == -1) temp_double = 0.0;
	else
	if (all_tokens[theta_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[theta_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[theta_column]);
		return false;
	}

	temp_parameters.theta = float(temp_double);

	// psi

	if (psi_column == -1) temp_double = 0.0;
	else
	if (all_tokens[psi_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[psi_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[psi_column]);
		return false;
	}

	temp_parameters.psi = float(temp_double);

	// xshift

	if (x_shift_column == -1) temp_double = 0.0;
	else
	if (all_tokens[x_shift_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[x_shift_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[x_shift_column]);
		return false;
	}

	temp_parameters.x_shift = float(temp_double);

	// yshift

	if (y_shift_column == -1) temp_double = 0.0;
	else
	if (all_tokens[y_shift_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[y_shift_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[y_shift_column]);
		return false;
	}

	temp_parameters.y_shift = float(temp_double);

	// defocus1

	if (all_tokens[defocus_1_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[defocus_1_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[defocus_1_column]);
		return false;
	}

	temp_parameters.defocus_1 = float(temp_double);

	// defocus2

	if (all_tokens[defocus_2_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[defocus_2_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[defocus_2_column]);
		return false;
	}

	temp_parameters.defocus_2 = float(temp_double);

	// defocus_angle

	if (all_tokens[defocus_angle_column].ToDouble(&temp_double) == false)
	{
		MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[defocus_angle_column]);
		if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[defocus_angle_column]);
		return false;
	}

	temp_parameters.defocus_angle = float(temp_double);

	// phase_shift

	if (phase_shift_column == -1) temp_parameters.phase_shift = 0.0;
	else
	{
		if (all_tokens[phase_shift_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[phase_shift_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[phase_shift_column]);
			return false;
		}

		temp_parameters.phase_shift = deg_2_rad(float(temp_double));
	}

	// occupancy

	if (occupancy_column == -1) temp_parameters.occupancy = 100.0f;
	else
	{
		if (all_tokens[occupancy_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[occupancy_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[occupancy_column]);
			return false;
		}

		temp_parameters.occupancy = float(temp_double);
	}

	// logp

	if (logp_column == -1) temp_parameters.logp = 100.0f;
	else
	{
		if (all_tokens[logp_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[logp_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[logp_column]);
			return false;
		}

		temp_parameters.logp = float(temp_double);
	}

	// sigma

	if (sigma_column == -1) temp_parameters.sigma = 10.0f;
	else
	{
		if (all_tokens[sigma_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[sigma_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[sigma_column]);
			return false;
		}

		temp_parameters.sigma = float(temp_double);
	}

	// score

	if (score_column == -1) temp_parameters.score = 0.0f;
	else
	{
		if (all_tokens[score_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[score_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[score_column]);
			return false;
		}

		temp_parameters.score = float(temp_double);
		//wxPrintf("Score = %f, image_is_active = %i, exclude negative = %s\n", temp_parameters.score, temp_parameters.image_is_active, BoolToYesNo(exclude_negative_film_numbers));
	}

	// score_change

	if (score_change_column == -1) temp_parameters.score_change = 0.0f;
	else
	{
		if (all_tokens[score_change_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[score_change_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[score_change_column]);
			return false;
		}

		temp_parameters.score_change = float(temp_double);
	}

	// pixel_size

	if (pixel_size_column == -1) temp_parameters.pixel_size = 0.0f;
	else
	{
		if (all_tokens[pixel_size_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[pixel_size_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[pixel_size_column]);
			return false;
		}

		temp_parameters.pixel_size = float(temp_double);
	}

	// voltage

	if (microscope_voltage_kv_column == -1) temp_parameters.microscope_voltage_kv = 0.0f;
	else
	{
		if (all_tokens[microscope_voltage_kv_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[microscope_voltage_kv_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[microscope_voltage_kv_column]);
			return false;
		}

		temp_parameters.microscope_voltage_kv = float(temp_double);
	}

	// Cs

	if ( microscope_spherical_aberration_mm_column == -1) temp_parameters.microscope_spherical_aberration_mm = 2.7f;
	else
	{
		if (all_tokens[microscope_spherical_aberration_mm_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[microscope_spherical_aberration_mm_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[microscope_spherical_aberration_mm_column]);
			return false;
		}

		temp_parameters.microscope_spherical_aberration_mm = float(temp_double);
	}

	// amplitude_contrast

	if ( amplitude_contrast_column == -1) temp_parameters.amplitude_contrast = 0.07f;
	else
	{
		if (all_tokens[amplitude_contrast_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[amplitude_contrast_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[amplitude_contrast_column]);
			return false;
		}

		temp_parameters.amplitude_contrast = float(temp_double);
	}

	// beam_tilt_x

	if ( beam_tilt_x_column == -1) temp_parameters.beam_tilt_x = 0.0f;
	else
	{
		if (all_tokens[beam_tilt_x_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[beam_tilt_x_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[beam_tilt_x_column]);
			return false;
		}

		temp_parameters.beam_tilt_x = float(temp_double);
	}

	// beam_tilt_y

	if ( beam_tilt_y_column == -1) temp_parameters.beam_tilt_y = 0.0f;
	else
	{
		if (all_tokens[beam_tilt_y_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[beam_tilt_y_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[beam_tilt_y_column]);
			return false;
		}

		temp_parameters.beam_tilt_y = float(temp_double);
	}

	// image_shift_x

	if ( image_shift_x_column == -1) temp_parameters.image_shift_x = 0.0f;
	else
	{
		if (all_tokens[image_shift_x_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[image_shift_x_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[image_shift_x_column]);
			return false;
		}

		temp_parameters.image_shift_x = float(temp_double);
	}

	// image_shift_y

	if ( image_shift_y_column == -1) temp_parameters.image_shift_y = 0.0f;
	else
	{
		if (all_tokens[image_shift_y_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[image_shift_y_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[image_shift_y_column]);
			return false;
		}

		temp_parameters.image_shift_y = float(temp_double);
	}

	// best 2D class

	if ( best_2d_class_column == -1) temp_parameters.best_2d_class = 0;
	else
	{
		if (all_tokens[best_2d_class_column].ToLong(&temp_long) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[best_2d_class_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[best_2d_class_column]);
			return false;
		}

		temp_parameters.best_2d_class = int(temp_long);
	}

	// beam tilt group

	if ( beam_tilt_group_column == -1) temp_parameters.beam_tilt_group = 0;
	else
	{
		if (all_tokens[beam_tilt_group_column].ToLong(&temp_long) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[beam_tilt_group_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[beam_tilt_group_column]);
			return false;
		}

		temp_parameters.beam_tilt_group = int(temp_long);
	}

	// particle group

	if ( particle_group_column == -1) temp_parameters.particle_group = 0;
	else
	{
		if (all_tokens[particle_group_column].ToLong(&temp_long) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[particle_group_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[particle_group_column]);
			return false;
		}

		temp_parameters.particle_group = int(temp_long);
	}

	// assigned subset (for half-dataset refinement, or half map FSCs, etc)

	if ( assigned_subset_column == -1) temp_parameters.assigned_subset = 0;
	else
	{
		if (all_tokens[assigned_subset_column].ToLong(&temp_long) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[assigned_subset_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[assigned_subset_column]);
			return false;
		}

		temp_parameters.assigned_subset = int(temp_long);
	}

	// pre exposure

	if ( pre_exposure_column == -1) temp_parameters.pre_exposure = 0.0f;
	else
	{
		if (all_tokens[pre_exposure_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[pre_exposure_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[pre_exposure_column]);
			return false;
		}

		temp_parameters.pre_exposure = float(temp_double);
	}

	// total exposure

	if ( total_exposure_column == -1) temp_parameters.total_exposure = 0.0f;
	else
	{
		if (all_tokens[total_exposure_column].ToDouble(&temp_double) == false)
		{
			MyPrintWithDetails("Error: Converting to a number (%s)\n", all_tokens[total_exposure_column]);
			if (error_string != NULL) *error_string = wxString::Format("Error: Converting to a number (%s)\n", all_tokens[total_exposure_column]);
			return false;
		}

		temp_parameters.total_exposure = float(temp_double);
	}


	// stack filename

	if ( stack_filename_column == -1) temp_parameters.stack_filename = "";
	else
	{
		temp_parameters.stack_filename = all_tokens[stack_filename_column].Trim(true).Trim(false);
		if (StripEnclosingSingleQuotesFromString(temp_parameters.stack_filename) == false)
		{
			MyPrintfRed("Error: stack file name read as %s is not enclosed in single quotes ('), replacing with blank string\n", temp_parameters.stack_filename);
			temp_parameters.stack_filename = "";
		}
	}

	// original_image_filename

	if ( original_image_filename_column == -1) temp_parameters.original_image_filename = "";
	else
	{
		temp_parameters.original_image_filename = all_tokens[original_image_filename_column].Trim(true).Trim(false);

		if (StripEnclosingSingleQuotesFromString(temp_parameters.original_image_filename) == false)
		{
			MyPrintfRed("Error: original image file name read as %s is not enclosed in single quotes ('), replacing with blank string\n", temp_parameters.original_image_filename);
			temp_parameters.original_image_filename = "";
		}
	}

	// reference_3d_filename

	if ( reference_3d_filename_column == -1) temp_parameters.reference_3d_filename = "";
	else
	{
		temp_parameters.reference_3d_filename = all_tokens[reference_3d_filename_column].Trim(true).Trim(false);

		if (StripEnclosingSingleQuotesFromString(temp_parameters.reference_3d_filename) == false)
		{
			MyPrintfRed("Error: reference 3d file name read as %s is not enclosed in single quotes ('), replacing with blank string\n", temp_parameters.reference_3d_filename);
			temp_parameters.reference_3d_filename = "";
		}
	}


	cached_parameters->Add(temp_parameters);

	return true;

}



bool cisTEMStarFileReader::ReadFile(wxString wanted_filename, wxString *error_string, ArrayOfcisTEMParameterLines *alternate_cached_parameters_pointer, bool exclude_negative_film_numbers)
{
	Open(wanted_filename, alternate_cached_parameters_pointer);
	wxString current_line;

	//MyDebugAssertTrue(input_file_stream != NULL, "FileStream is NULL!");
	MyDebugAssertTrue(input_file->IsOpened(), "File not open");

	bool found_valid_data_block = false;
	bool found_valid_loop_block = false;

	input_file->GoToLine(-1); // this triggers warning: integer conversion resulted in a change of sign
	// find a data block

	//while (input_file_stream->Eof() == false)
	while (input_file->Eof() == false)
	{
		//current_line = input_text_stream->ReadLine();
		current_line = input_file->GetNextLine();
		current_line = current_line.Trim(true);
		current_line = current_line.Trim(false);
		if (current_line.Find("data_") != wxNOT_FOUND)
		{
			found_valid_data_block = true;
			break;
		}
	}

	if (found_valid_data_block == false)
	{
		MyPrintWithDetails("Error: Couldn't find a valid data block in star file (%s)\n", wanted_filename);

		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find a valid data block in star file (%s)\n", wanted_filename);
		return false;
	}

	// find a loop block

	//while (input_file_stream->Eof() == false)
	while (input_file->Eof() == false)
	{
		//current_line = input_text_stream->ReadLine();
		current_line = input_file->GetNextLine();
		current_line = current_line.Trim(true);
		current_line = current_line.Trim(false);

		if (current_line.Find("loop_") != wxNOT_FOUND)
		{
			found_valid_loop_block = true;
			break;
		}
	}

	if (found_valid_loop_block == false)
	{
		MyPrintWithDetails("Error: Couldn't find a valid loop block in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find a valid loop block in star file (%s)\n", wanted_filename);
		return false;
	}

	// now we can get headers..

	ResetColumnPositions();

	//while (input_file_stream->Eof() == false)
	while (input_file->Eof() == false)
	{
		//current_line = input_text_stream->ReadLine();
		current_line = input_file->GetNextLine();
		current_line = current_line.Trim(true);
		current_line = current_line.Trim(false);

	    if (current_line[0] == '#' || current_line[0] == '\0' || current_line[0] == ';') continue;
	    if (current_line[0] != '_') break;

	    // otherwise it is a label, is it a label we want though?

	    if (current_line.StartsWith("_cisTEMPositionInStack ") == true)
	    {
	    	if (position_in_stack_column != -1) wxPrintf("Warning :: _cisTEMPositionInStack occurs more than once. I will take the last occurrence\n");
	    	position_in_stack_column = current_column;
	    	parameters_that_were_read.position_in_stack = true;
	    }
	    else
	    if (current_line.StartsWith("_cisTEMAnglePsi ") == true)
	    {
	    	if (psi_column != -1) wxPrintf("Warning :: _cisTEMAnglePsi occurs more than once. I will take the last occurrence\n");
		   	psi_column = current_column;
			parameters_that_were_read.psi = true;
	    }
	    else
		if (current_line.StartsWith("_cisTEMAngleTheta ") == true)
		{
	    	if (theta_column != -1) wxPrintf("Warning :: _cisTEMAngleTheta occurs more than once. I will take the last occurrence\n");
			theta_column = current_column;
			parameters_that_were_read.theta = true;
		}
		else
		if (current_line.StartsWith("_cisTEMAnglePhi ") == true)
		{
	    	if (phi_column != -1) wxPrintf("Warning :: _cisTEMAnglePhi occurs more than once. I will take the last occurrence\n");
		   	phi_column = current_column;
			parameters_that_were_read.phi = true;
		}
		else
		if (current_line.StartsWith("_cisTEMXShift ") == true)
		{
	    	if (x_shift_column != -1) wxPrintf("Warning :: _cisTEMXShift occurs more than once. I will take the last occurrence\n");
		   	x_shift_column = current_column;
			parameters_that_were_read.x_shift = true;
		}
		else
		if (current_line.StartsWith("_cisTEMYShift ") == true)
		{
	    	if (y_shift_column != -1) wxPrintf("Warning :: _cisTEMYShift occurs more than once. I will take the last occurrence\n");
		   	y_shift_column = current_column;
			parameters_that_were_read.y_shift = true;
		}
		else
		if (current_line.StartsWith("_cisTEMDefocus1 ") == true)
		{
	    	if (defocus_1_column != -1) wxPrintf("Warning :: _cisTEMDefocus1 occurs more than once. I will take the last occurrence\n");
		   	defocus_1_column = current_column;
			parameters_that_were_read.defocus_1 = true;
		}
		else
		if (current_line.StartsWith("_cisTEMDefocus2 ") == true)
		{
	    	if (defocus_2_column != -1) wxPrintf("Warning :: _cisTEMDefocus2 occurs more than once. I will take the last occurrence\n");
		   	defocus_2_column = current_column;
			parameters_that_were_read.defocus_2 = true;
		}
		else
		if (current_line.StartsWith("_cisTEMDefocusAngle ") == true)
		{
	    	if (defocus_angle_column != -1) wxPrintf("Warning :: _cisTEMDefocusAngle occurs more than once. I will take the last occurrence\n");
		   	defocus_angle_column = current_column;
			parameters_that_were_read.defocus_angle = true;
		}
		else
		if (current_line.StartsWith("_cisTEMPhaseShift ") == true)
		{
	    	if (phase_shift_column != -1) wxPrintf("Warning :: _cisTEMPhaseShift occurs more than once. I will take the last occurrence\n");
		   	phase_shift_column = current_column;
			parameters_that_were_read.phase_shift = true;
		}
		else
		if (current_line.StartsWith("_cisTEMImageActivity ") == true)
		{
	    	if (image_is_active_column != -1) wxPrintf("Warning :: _cisTEMImageActivity occurs more than once. I will take the last occurrence\n");
		   	image_is_active_column = current_column;
			parameters_that_were_read.image_is_active = true;
		}
		else
		if (current_line.StartsWith("_cisTEMOccupancy ") == true)
		{
	    	if (occupancy_column != -1) wxPrintf("Warning :: _cisTEMOccupancy occurs more than once. I will take the last occurrence\n");
		   	occupancy_column = current_column;
			parameters_that_were_read.occupancy = true;
		}
		else
		if (current_line.StartsWith("_cisTEMLogP ") == true)
		{
	    	if (logp_column != -1) wxPrintf("Warning :: _cisTEMLogP occurs more than once. I will take the last occurrence\n");
		   	logp_column = current_column;
			parameters_that_were_read.logp = true;
		}
		else
		if (current_line.StartsWith("_cisTEMSigma ") == true)
		{
	    	if (sigma_column != -1) wxPrintf("Warning :: _cisTEMSigma occurs more than once. I will take the last occurrence\n");
		   	sigma_column = current_column;
		   	parameters_that_were_read.sigma = true;
		}
		else
		if (current_line.StartsWith("_cisTEMScore ") == true)
		{
	    	if (score_column != -1) wxPrintf("Warning :: _cisTEMScore occurs more than once. I will take the last occurrence\n");
		   	score_column = current_column;
			parameters_that_were_read.score = true;
		}
		else
		if (current_line.StartsWith("_cisTEMScoreChange ") == true)
		{
	    	if (score_change_column != -1) wxPrintf("Warning :: _cisTEMScoreChange occurs more than once. I will take the last occurrence\n");
		   	score_change_column = current_column;
			parameters_that_were_read.score_change = true;
		}
		else
		if (current_line.StartsWith("_cisTEMPixelSize ") == true)
		{
	    	if (pixel_size_column != -1) wxPrintf("Warning :: _cisTEMPixelSize occurs more than once. I will take the last occurrence\n");
		   	pixel_size_column = current_column;
			parameters_that_were_read.pixel_size = true;
		}
		else
		if (current_line.StartsWith("_cisTEMMicroscopeVoltagekV ") == true)
		{
	    	if (microscope_voltage_kv_column != -1) wxPrintf("Warning :: _cisTEMMicroscopeVoltagekV occurs more than once. I will take the last occurrence\n");
		   	microscope_voltage_kv_column = current_column;
			parameters_that_were_read.microscope_voltage_kv = true;
		}
		else
		if (current_line.StartsWith("_cisTEMMicroscopeCsMM ") == true)
		{
	    	if (microscope_spherical_aberration_mm_column != -1) wxPrintf("Warning :: _cisTEMMicroscopeCsMM occurs more than once. I will take the last occurrence\n");
		   	microscope_spherical_aberration_mm_column = current_column;
			parameters_that_were_read.microscope_spherical_aberration_mm = true;
		}
		else
		if (current_line.StartsWith("_cisTEMAmplitudeContrast ") == true)
		{
	    	if (amplitude_contrast_column != -1) wxPrintf("Warning :: _cisTEMAmplitudeContrast occurs more than once. I will take the last occurrence\n");
		   	amplitude_contrast_column = current_column;
			parameters_that_were_read.amplitude_contrast = true;
		}
		else
		if (current_line.StartsWith("_cisTEMBeamTiltX ") == true)
		{
	    	if (beam_tilt_x_column != -1) wxPrintf("Warning :: _cisTEMBeamTiltX occurs more than once. I will take the last occurrence\n");
		   	beam_tilt_x_column = current_column;
			parameters_that_were_read.beam_tilt_x = true;
		}
		else
		if (current_line.StartsWith("_cisTEMBeamTiltY ") == true)
		{
	    	if (beam_tilt_y_column != -1) wxPrintf("Warning :: _cisTEMBeamTiltY occurs more than once. I will take the last occurrence\n");
		   	beam_tilt_y_column = current_column;
			parameters_that_were_read.beam_tilt_y = true;
		}
		else
		if (current_line.StartsWith("_cisTEMImageShiftX ") == true)
		{
	    	if (image_shift_x_column != -1) wxPrintf("Warning :: _cisTEMImageShiftX occurs more than once. I will take the last occurrence\n");
		   	image_shift_x_column = current_column;
			parameters_that_were_read.image_shift_x = true;
		}
		else
		if (current_line.StartsWith("_cisTEMImageShiftY ") == true)
		{
	    	if (image_shift_y_column != -1) wxPrintf("Warning :: _cisTEMImageShiftY occurs more than once. I will take the last occurrence\n");
		   	image_shift_y_column = current_column;
			parameters_that_were_read.image_shift_y = true;
		}
		else
		if (current_line.StartsWith("_cisTEMBest2DClass ") == true)
		{
	    	if (best_2d_class_column != -1) wxPrintf("Warning :: _cisTEMBest2DClass occurs more than once. I will take the last occurrence\n");
		   	best_2d_class_column = current_column;
			parameters_that_were_read.best_2d_class = true;
		}
		else
		if (current_line.StartsWith("_cisTEMBeamTiltGroup ") == true)
		{
	    	if (beam_tilt_group_column != -1) wxPrintf("Warning :: _cisTEMBeamTiltGroup occurs more than once. I will take the last occurrence\n");
		   	beam_tilt_group_column = current_column;
			parameters_that_were_read.beam_tilt_group = true;
		}
		else
		if (current_line.StartsWith("_cisTEMParticleGroup ") == true)
		{
	    	if (particle_group_column != -1) wxPrintf("Warning :: _cisTEMParticleGroup occurs more than once. I will take the last occurrence\n");
		   	particle_group_column = current_column;
			parameters_that_were_read.particle_group = true;
		}
		else
		if (current_line.StartsWith("_cisTEMAssignedSubset ") == true)
		{
	    	if (assigned_subset_column != -1) wxPrintf("Warning :: _cisTEMAssignedSubset occurs more than once. I will take the last occurrence\n");
		   	assigned_subset_column = current_column;
			parameters_that_were_read.assigned_subset = true;
		}
		else
		if (current_line.StartsWith("_cisTEMPreExposure ") == true)
		{
	    	if (pre_exposure_column != -1) wxPrintf("Warning :: _cisTEMPreExposure occurs more than once. I will take the last occurrence\n");
		   	pre_exposure_column = current_column;
			parameters_that_were_read.pre_exposure = true;
		}
		else
		if (current_line.StartsWith("_cisTEMTotalExposure ") == true)
		{
	    	if (total_exposure_column != -1) wxPrintf("Warning :: _cisTEMTotalExposure occurs more than once. I will take the last occurrence\n");
		   	total_exposure_column = current_column;
			parameters_that_were_read.total_exposure = true;
		}
		else
		if (current_line.StartsWith("_cisTEMReference3DFilename ") == true)
		{
	    	if (reference_3d_filename_column != -1) wxPrintf("Warning :: _cisTEMReference3DFilename occurs more than once. I will take the last occurrence\n");
		   	reference_3d_filename_column = current_column;
			parameters_that_were_read.reference_3d_filename = true;
		}
		else
		if (current_line.StartsWith("_cisTEMOriginalImageFilename ") == true)
		{
	    	if (original_image_filename_column != -1) wxPrintf("Warning :: _cisTEMOriginalImageFilename occurs more than once. I will take the last occurrence\n");
		   	original_image_filename_column = current_column;
			parameters_that_were_read.original_image_filename = true;
		}
		else
		if (current_line.StartsWith("_cisTEMReference3DFilename ") == true)
		{
	    	if (reference_3d_filename_column != -1) wxPrintf("Warning :: _cisTEMReference3DFilename occurs more than once. I will take the last occurrence\n");
		   	reference_3d_filename_column = current_column;
			parameters_that_were_read.reference_3d_filename = true;
		}

	    current_column++;
	}

	// quick checks we have all the desired info.
/*
	if (phi_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnAngleRot in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnAngleRot in star file (%s)\n", wanted_filename);
		return false;
	}

	if (theta_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnAngleTilt in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnAngleTilt in star file (%s)\n", wanted_filename);
		return false;
	}

	if (psi_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnAnglePsi in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnAnglePsi in star file (%s)\n", wanted_filename);
		return false;
	}

	if (xshift_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnOriginX in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnOriginX in star file (%s)\n", wanted_filename);
		return false;
	}

	if (yshift_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnOriginY in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnOriginY in star file (%s)\n", wanted_filename);
		return false;
	}

	if (defocus1_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnDefocusU in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnDefocusU in star file (%s)\n", wanted_filename);
		return false;
	}

	if (defocus2_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnDefocusV in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnDefocusV in star file (%s)\n", wanted_filename);
		return false;
	}

	if (defocus_angle_column == -1)
	{
		MyPrintWithDetails("Error: Couldn't find _rlnDefocusAngle in star file (%s)\n", wanted_filename);
		if (error_string != NULL) *error_string = wxString::Format("Error: Couldn't find _rlnDefocusAngle in star file (%s)\n", wanted_filename);
		return false;
	}

	if (phase_shift_column == -1)
	{
	//	MyPrintWithDetails("Warning: Couldn't find _rlnPhaseShift in star file (%s) - phase shift will be set to 0.0\n", wanted_filename);
	}

*/

	// we have the headers, the current line should be the first parameter to extract the info

	if (ExtractParametersFromLine(current_line, error_string, exclude_negative_film_numbers) == false) return false;

	// loop over the data lines and fill in..

	//while (input_file_stream->Eof() == false)
	while (input_file->Eof() == false)
	{
		//current_line = input_text_stream->ReadLine();
		current_line = input_file->GetNextLine();
		current_line = current_line.Trim(true);
		current_line = current_line.Trim(false);

		if (current_line.IsEmpty() == true) break;
		if (current_line[0] == '#' || current_line[0] == '\0' || current_line[0] == ';') continue;

		if (ExtractParametersFromLine(current_line, error_string, exclude_negative_film_numbers) == false) return false;
	}

	return true;
}

