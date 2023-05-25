#include "Fusion3D.h"
#include "extractor_api.h"
#include <QMessageBox>
#include <QFileDialog>
#include <string>
#include <tchar.h>
#define IMG_PATH_ERROR 0
#define SCENE_PATH_ERROR 1
#define SAMPLE_LEN_ERROR 2
#define RECON_DONE       3
#define MSG_FROM_RECON   4
#define SCALE_ERROR 5
#define MAX_PIXELS1  3000000
#define MAX_PIXELS2  6000000
Fusion3D* pMainWnd = NULL;

void On3DProgress(int total_progress, int sub_progress, char* msg) {
	
	emit(pMainWnd->progressReady(total_progress, sub_progress, msg));
}

Fusion3D::Fusion3D(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.btnExecute, SIGNAL(clicked()), this, SLOT(onBtnStartClick()));
	connect(ui.btnImagePath, SIGNAL(clicked()), this, SLOT(onBtnBrowseImagePath()));
	connect(ui.btnOutScene, SIGNAL(clicked()), this, SLOT(onBtnBrowseOutPath()));
	connect(ui.akaze_check, SIGNAL(clicked()), this, SLOT(onAkazeCheck()));
	connect(ui.sift_check, SIGNAL(clicked()), this, SLOT(onSiftCheck()));
	connect(ui.akaze_sift_check, SIGNAL(clicked()), this, SLOT(onAkaze_SiftCheck()));
	connect(ui.mvs_check, SIGNAL(clicked()), this, SLOT(onMvsCheck()));
	connect(ui.smvs_check, SIGNAL(clicked()), this, SLOT(onSMvsCheck()));
	connect(ui.seperate_recon_check, SIGNAL(clicked()), this, SLOT(onSeperateRecon()));
	connect(ui.openModelBtn, SIGNAL(clicked()), this, SLOT(onOpenModelClick()));
	connect(this, &Fusion3D::progressReady, this, &Fusion3D::setProgress);
	connect(ui.default_setting_check, SIGNAL(clicked()), this, SLOT(onDefaultCheck()));
	connect(ui.scene_btn, SIGNAL(clicked()), this, SLOT(onMakeSceneBtn()));
	connect(ui.sfm_btn, SIGNAL(clicked()), this, SLOT(onSfmReconBtn()));
	connect(ui.dm_btn, SIGNAL(clicked()), this, SLOT(onDmReconBtn()));
	connect(ui.fss_btn, SIGNAL(clicked()), this, SLOT(onFssReconBtn()));
	connect(ui.tex_btn, SIGNAL(clicked()), this, SLOT(onTexReconBtn()));
	connect(ui.tex_scale_btn, SIGNAL(clicked()), this, SLOT(onScalingTexMesh()));
	pMainWnd = this;
	//setVisibleOfCtrls(true);
	dispLabel(-1);
	setDefault();
}

void Fusion3D::onDefaultCheck()
{
	if (ui.default_setting_check->isChecked())
	{
		setDefault();
	}
	else
	{
		ui.akaze_check->setDisabled(false);
		ui.akaze_sift_check->setDisabled(false);
		//dm mode
		ui.mvs_check->setDisabled(false);
		ui.smvs_check->setDisabled(false);
		ui.use_shade_check->setDisabled(false);
		//image size, dm input scale, dm out scale
		ui.img_size_spin->setDisabled(false);
		ui.input_scale_spin->setDisabled(false);
		ui.out_scale_spin->setDisabled(false);
	}
}

void Fusion3D::setVisibleOfCtrls(bool flg)
{
	ui.akaze_check->setVisible(flg);
	ui.akaze_sift_check->setVisible(flg);
	ui.label_2->setVisible(flg);
	ui.label_3->setVisible(flg);
	ui.label_5->setVisible(flg);
	ui.plane_remove_check->setVisible(flg);
	ui.mvs_check->setVisible(flg);
	ui.plane_tolerance_edt->setVisible(flg);
	ui.sift_check->setVisible(flg);
	ui.smvs_check->setVisible(flg);
	ui.img_size_spin->setVisible(flg);
	ui.msg_from_recon_label->setVisible(flg);
	ui.scale_err_label->setVisible(flg);
}

void Fusion3D::setDefault()
{
	ui.default_setting_check->setChecked(true);
	//feature setting
	ui.akaze_check->setChecked(false);
	ui.akaze_check->setDisabled(true);
	ui.akaze_sift_check->setChecked(false);
	ui.akaze_sift_check->setDisabled(true);
	ui.sift_check->setChecked(true);
	//dm mode
	ui.mvs_check->setChecked(false);
	ui.mvs_check->setDisabled(true);
	ui.smvs_check->setChecked(true);
	ui.smvs_check->setDisabled(true);
	ui.use_shade_check->setChecked(false);
	ui.use_shade_check->setDisabled(true);
	//image size, dm input scale, dm out scale
	ui.img_size_spin->setValue(2);
	ui.img_size_spin->setDisabled(true);
	ui.input_scale_spin->setValue(3);
	ui.input_scale_spin->setDisabled(true);
	ui.out_scale_spin->setValue(3);
	ui.out_scale_spin->setDisabled(true);
	//step buttons should be disable
	ui.dm_btn->setDisabled(true);
	ui.sfm_btn->setDisabled(true);
	ui.fss_btn->setDisabled(true);
	ui.scene_btn->setDisabled(true);
	ui.tex_btn->setDisabled(true);
}

void Fusion3D::onSeperateRecon()
{
	if (ui.seperate_recon_check->isChecked())
	{
		ui.dm_btn->setDisabled(false);
		ui.sfm_btn->setDisabled(false);
		ui.fss_btn->setDisabled(false);
		ui.scene_btn->setDisabled(false);
		ui.tex_btn->setDisabled(false);
		ui.btnExecute->setDisabled(true);
		initReconSetting();
	}
	else
	{
		ui.btnExecute->setDisabled(false);
		ui.dm_btn->setDisabled(true);
		ui.sfm_btn->setDisabled(true);
		ui.fss_btn->setDisabled(true);
		ui.scene_btn->setDisabled(true);
		ui.tex_btn->setDisabled(true);
	}
}

void Fusion3D::setProgress(int total, int sub, char* msg)
{
	ui.msg_from_recon_label->setVisible(true);
	QString qstr;
	qstr.fromLocal8Bit(msg);
	ui.msg_from_recon_label->setText(qstr);
	ui.progressBar_total->setValue(total);
	ui.progressBar_sub->setValue(sub);
}

void Fusion3D::onOpenModelClick()
{
	disableControlStates(false);
	QString dirScene = ui.label_outpath->text();
	if (dirScene == "")
	{
		dispLabel(SCENE_PATH_ERROR);
		return;
	}
	std::string strScenePath = dirScene.toUtf8().constData();
	QString str = ui.sample_len_edt->text();
	float sample_len = str.toFloat();
	if (sample_len == 0.0f)
	{
		//dispLabel(SAMPLE_LEN_ERROR);
		return;
	}
	ui.btnExecute->setDisabled(true);
	//disableControlStates(true);
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Model"), "", tr("model File (*.ply)"));
	if (fileName == "")
		return;
	std::string scaled_path = strScenePath + "/mesh_scaled.ply";
	std::string decimate_path = strScenePath + "/mesh_decimated.ply";
	setting(sample_len, scaled_path, decimate_path);
	int nRes = loadScene(strScenePath);
	switch (nRes)
	{
	case 0:
		return;
	case 1:
		return;
	}
	loadMesh(fileName.toStdString());
	std::string caption = "viewer";
	createViewer(10, 10, 1280, 960, caption);
	drawScene(1);
}

void Fusion3D::onBtnBrowseImagePath() {
	QString dirImage = QFileDialog::getExistingDirectory(this, tr("Image Directory"),
		"d:/9_work/2022/3d_construct/",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	ui.label_ImagePath->setText(dirImage);
}

void Fusion3D::onBtnBrowseOutPath() {
	QString dirScene = QFileDialog::getExistingDirectory(this, tr("Scene Directory"),
		"d:/9_work/2022/3d_construct/",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	ui.label_outpath->setText(dirScene);
}

bool Fusion3D::initReconSetting()
{
	QString dirImage = ui.label_ImagePath->text();
	QString dirScene = ui.label_outpath->text();
	QString tmp("알림");
	if (dirImage == "") {
		//QMessageBox::warning(this, tmp, QString::fromStdString("화상경로를 입력하십시오."), tr("확인"));
		//ui.img_path_msg_label->setVisible(true);
		dispLabel(IMG_PATH_ERROR);
		return false;
	}
	if (dirScene == "") {
		dispLabel(SCENE_PATH_ERROR);
		return false;
	}
	std::string strImagePath = dirImage.toUtf8().constData();
	std::string strScenePath = dirScene.toUtf8().constData();
	int matchMode = 3;//0:cv_akaze plus cv_sift feature matching, 1:cv_akaze feature matching, 2:cv_sift feature matching, 3:mve feature matching
	int mvsType = 1;//0:mvs, 1:smvs
	if (ui.sift_check->isChecked() == true)
		matchMode = 3;
	if (ui.akaze_check->isChecked() == true)
		matchMode = 1;
	if (ui.akaze_sift_check->isChecked() == true)
		matchMode = 0;
	if (ui.mvs_check->isChecked() == true)//.mvs_check->isChecked() == true)
		mvsType = 0;
	if (ui.smvs_check->isChecked() == true)
		mvsType = 1;
	int img_size_level = ui.img_size_spin->value();
	int dm_input_scale = ui.input_scale_spin->value();
	int dm_out_scale = ui.out_scale_spin->value();
	QString str = ui.plane_tolerance_edt->text();
	float plane_tolerance = str.toFloat();
	if (ui.plane_remove_check->isChecked() == false)
		plane_tolerance = 0.0f;
	float cluster_tolerance = 0.0f;
	bool use_shading = ui.use_shade_check->isChecked();
	init_recon(strImagePath, strScenePath, matchMode, mvsType, img_size_level, dm_input_scale, dm_out_scale, use_shading,
		plane_tolerance, cluster_tolerance, (On3DProgressCallback)On3DProgress);
	return true;
}

void Fusion3D::onBtnStartClick()
{
	QString dirImage = ui.label_ImagePath->text();
	QString dirScene = ui.label_outpath->text();
	QString tmp("알림");
	if (dirImage == "") {
		//QMessageBox::warning(this, tmp, QString::fromStdString("화상경로를 입력하십시오."), tr("확인"));
		dispLabel(IMG_PATH_ERROR);
		return;
	}
	if (dirScene == "") {
		dispLabel(SCENE_PATH_ERROR);
		return; 
	}
	std::string strImagePath = dirImage.toUtf8().constData();
	std::string strScenePath = dirScene.toUtf8().constData();
	int matchMode = 3;//0:cv_akaze plus cv_sift feature matching, 1:cv_akaze feature matching, 2:cv_sift feature matching, 3:mve feature matching
	int mvsType = 1;//0:mvs, 1:smvs
	if (ui.sift_check->isChecked() == true)
		matchMode = 3;
	if (ui.akaze_check->isChecked() == true)
		matchMode = 1;
	if (ui.akaze_sift_check->isChecked() == true)
		matchMode = 0;
	if (ui.mvs_check->isChecked() == true)//.mvs_check->isChecked() == true)
		mvsType = 0;
	if (ui.smvs_check->isChecked() == true)
		mvsType = 1;
	int img_size_level = ui.img_size_spin->value();
	int dm_input_scale = ui.input_scale_spin->value();
	int dm_out_scale = ui.out_scale_spin->value();
	QString str = ui.plane_tolerance_edt->text();
	float plane_tolerance = str.toFloat();
	if (ui.plane_remove_check->isChecked() == false)
		plane_tolerance = 0.0f;
	float cluster_tolerance = 0.0f;
	bool use_shading = ui.use_shade_check->isChecked();
	init_recon(strImagePath, strScenePath, matchMode, mvsType, img_size_level, dm_input_scale, dm_out_scale, use_shading,
			plane_tolerance, cluster_tolerance, (On3DProgressCallback)On3DProgress);
	ui.btnExecute->setEnabled(false);
	WorkerThread *reconn_thread = new WorkerThread(strImagePath, strScenePath, matchMode, mvsType);
	connect(reconn_thread, &WorkerThread::resultReady, this, &Fusion3D::onReconFinished);
	reconn_thread->start();
	disableControlStates(true);
	//recon_all(strImagePath, strScenePath, matchMode, mvsType);
}

void Fusion3D::onReconFinished() {
	disableControlStates(false);
	dispLabel(RECON_DONE);
}

void Fusion3D::onAkazeCheck()
{
	ui.sift_check->setChecked(false);
	ui.akaze_sift_check->setChecked(false);
}

void Fusion3D::onSiftCheck()
{
	ui.akaze_check->setChecked(false);
	ui.akaze_sift_check->setChecked(false);
}

void Fusion3D::onAkaze_SiftCheck()
{
	ui.sift_check->setChecked(false);
	ui.akaze_check->setChecked(false);
}

void Fusion3D::onMvsCheck()
{
	ui.smvs_check->setChecked(false);
}

void Fusion3D::onSMvsCheck()
{
	ui.mvs_check->setChecked(false);
}

void Fusion3D::disableControlStates(bool flg)
{
	ui.akaze_check->setDisabled(flg);
	ui.akaze_sift_check->setDisabled(flg);
	ui.btnExecute->setDisabled(flg);
	ui.btnImagePath->setDisabled(flg);
	ui.btnOutScene->setDisabled(flg);
	ui.mvs_check->setDisabled(flg);
	ui.openModelBtn->setDisabled(flg);
	ui.plane_remove_check->setDisabled(flg);
	ui.plane_tolerance_edt->setDisabled(flg);
	ui.sample_len_edt->setDisabled(flg);
	ui.sift_check->setDisabled(flg);
	ui.smvs_check->setDisabled(flg);
	ui.img_size_spin->setDisabled(flg);
	ui.input_scale_spin->setDisabled(flg);
	ui.out_scale_spin->setDisabled(flg);
	ui.default_setting_check->setDisabled(flg);
}

void Fusion3D::dispLabel(int nLabel)
{
	ui.img_path_msg_label->setVisible(false);
	ui.scene_path_msg_label->setVisible(false);
	ui.ply_path_error_label->setVisible(false);
	ui.sample_len_err_label->setVisible(false);
	ui.recon_done_label->setVisible(false);
	ui.msg_from_recon_label->setVisible(false);
	ui.scale_err_label->setVisible(false);
	if (nLabel == -1)
		return;
	switch (nLabel)
	{
	case IMG_PATH_ERROR:
		ui.img_path_msg_label->setVisible(true);
		break;
	case SCENE_PATH_ERROR:
		ui.scene_path_msg_label->setVisible(true);
		break;
	case RECON_DONE:
		ui.recon_done_label->setVisible(true);
		break;
	case MSG_FROM_RECON:
		ui.msg_from_recon_label->setVisible(true);
		break;
	case SAMPLE_LEN_ERROR:
		ui.sample_len_err_label->setVisible(true);
		break;
	case SCALE_ERROR:
		ui.scale_err_label->setVisible(true);
	}
}

void Fusion3D::onMakeSceneBtn()
{
	make_scene();
}

void Fusion3D::onSfmReconBtn()
{
	sfm_recon();
}

void Fusion3D::onDmReconBtn()
{
	int mvsType = 0;
	if (ui.mvs_check->isChecked() == true)//.mvs_check->isChecked() == true)
		mvsType = 0;
	if (ui.smvs_check->isChecked() == true)
		mvsType = 1;
	dm_recon(mvsType);
}

void Fusion3D::onFssReconBtn()
{
	fss_recon();
}

void Fusion3D::onTexReconBtn()
{
	QString dirScene = ui.label_outpath->text();
	if (dirScene == "") {
		dispLabel(SCENE_PATH_ERROR);
		return;
	}
	std::string scene_path = dirScene.toUtf8().constData();
	tex_recon(scene_path, 0.0f);
}

void Fusion3D::onScalingTexMesh()
{
	float scale = getScale();
	if (scale == 0.0f)
	{
		dispLabel(SCALE_ERROR);
		return;
	}
	QString dirScene = ui.label_outpath->text();
	if (dirScene == "") {
		dispLabel(SCENE_PATH_ERROR);
		return;
	}
	std::string scene_path = dirScene.toUtf8().constData();
	tex_recon(scene_path, scale);
	QMessageBox::warning(this, QString::fromStdString("scaling tex mesh."), QString::fromStdString("scaling tex mesh is done."), tr("ok"));
}