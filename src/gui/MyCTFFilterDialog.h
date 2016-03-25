#ifndef __MyMovieFilterDialog__
#define __MyMovieFilterDialog__

#include "ProjectX_gui.h"

class MyFilterDialog : public FilterDialog
{
public :

	MyFilterDialog (wxWindow *parent);

	void SizeAndPosition();
};


class MyMovieFilterDialog : public MyFilterDialog
{
	public:

		MyMovieFilterDialog( wxWindow* parent );

		void OnCancelClick( wxCommandEvent& event );
		void OnFilterClick( wxCommandEvent& event );
		int ReturnNumberChecked();
		void BuildSearchCommand();

		wxString search_command;

		IntegerFilterItem *asset_id_filter;
		IntegerFilterItem *alignment_id_filter;
		DateFilterItem *date_of_run_filter;
		IntegerFilterItem *job_id_filter;
		FloatFilterItem *voltage_filter;
		FloatFilterItem *pixel_size_filter;
		FloatFilterItem *exposure_per_frame_filter;
		FloatFilterItem *pre_exposure_filter;

		wxRadioButton* AssetIDRadioButton;
		wxRadioButton* AlignmentIDRadioButton;
		wxRadioButton* DateOfRunRadioButton;
		wxRadioButton* JobIDRadioButton;
		wxRadioButton* VoltageRadioButton;
		wxRadioButton* PixelSizeRadioButton;
		wxRadioButton* ExposureRadioButton;
		wxRadioButton* PreExposureRadioButton;
};

class MyCTFFilterDialog : public MyFilterDialog
{
public:

	MyCTFFilterDialog( wxWindow* parent );

	void OnCancelClick( wxCommandEvent& event );
	void OnFilterClick( wxCommandEvent& event );
	int ReturnNumberChecked();
	void BuildSearchCommand();

	wxString search_command;

	IntegerFilterItem *asset_id_filter;
	IntegerFilterItem *estimation_id_filter;
	DateFilterItem *date_of_run_filter;
	IntegerFilterItem *job_id_filter;
	FloatFilterItem *defocus;
	FloatFilterItem *astigmatism;
	FloatFilterItem *astigmatism_angle;
	FloatFilterItem *score;
	FloatFilterItem *ring_resolution;
	FloatFilterItem *alias_resolution;
	FloatFilterItem *additional_phase_shift;
	FloatFilterItem *voltage_filter;
	FloatFilterItem *spherical_aberation_filter;
	FloatFilterItem *pixel_size_filter;
	FloatFilterItem *amplitude_contrast_filter;
	IntegerFilterItem *box_size_filter;
	FloatFilterItem *min_resolution_filter;
	FloatFilterItem *max_resolution_filter;
	FloatFilterItem *min_defocus_filter;
	FloatFilterItem *max_defocus_filter;
	FloatFilterItem *defocus_step_filter;
	FloatFilterItem *tolerated_astigmatism_filter;
	FloatFilterItem *min_phase_shift_filter;
	FloatFilterItem *max_phase_shift_filter;
	FloatFilterItem *phase_shift_step_filter;

	wxRadioButton *asset_id_radio;
	wxRadioButton *estimation_id_radio;
	wxRadioButton *date_radio;
	wxRadioButton *job_id_radio;
	wxRadioButton *defocus_radio;
	wxRadioButton *astigmatism_radio;
	wxRadioButton *astigmatism_angle_radio;
	wxRadioButton *score_radio;
	wxRadioButton *ring_resolution_radio;
	wxRadioButton *alias_resolution_radio;
	wxRadioButton *additional_phase_shift_radio;
	wxRadioButton *voltage_radio;
	wxRadioButton *spherical_abeeration_radio;
	wxRadioButton *pixel_size_radio;
	wxRadioButton *amplitude_contrast_radio;
	wxRadioButton *box_size_radio;
	wxRadioButton *min_resolution_radio;
	wxRadioButton *max_resolution_radio;
	wxRadioButton *min_defocus_radio;
	wxRadioButton *max_defocus_radio;
	wxRadioButton *defocus_step_radio;
	wxRadioButton *tolerated_astigmatism_radio;
	wxRadioButton *min_phase_shift_radio;
	wxRadioButton *max_phase_shift_radio;
	wxRadioButton *phase_shift_step_radio;


};

#endif // __MyMovieFilterDialog__
