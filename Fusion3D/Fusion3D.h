#pragma once

#include <QtWidgets/QMainWindow>
#include <qthread.h>
#include "ui_Fusion3D.h"
#include <Recon_API.h>

void On3DProgress(int total_progress, int sub_progress, char* msg);

class WorkerThread : public QThread
{
private:
	std::string m_strImagePath;
	std::string m_strScenePath;
	int m_nMatchType, m_nMvsType;
public:
	WorkerThread(std::string &imgPath, std::string &scenePath, int matchType, int mvsType) {
		m_strImagePath = imgPath;
		m_strScenePath = scenePath;
		m_nMatchType = matchType;
		m_nMvsType = mvsType;
	}

	Q_OBJECT
		void run() override {
		QString result;
		/* ... here is the expensive or blocking operation ... */
		recon_all(m_strImagePath, m_strScenePath, m_nMatchType, m_nMvsType);
		emit resultReady();
	}
signals:
	void resultReady();
};

class Fusion3D : public QMainWindow
{
	Q_OBJECT

public:
	Fusion3D(QWidget *parent = Q_NULLPTR);
	
private:
	bool initReconSetting();
	void disableControlStates(bool flg);
	void dispLabel(int nLabel);
	void setVisibleOfCtrls(bool flg);
	void setDefault();
private:
	Ui::Fusion3DClass ui;
	QString plyFileName;
public:

signals:
void progressReady(int total, int sub, char* msg);

public slots:
	void onBtnStartClick();
	void onBtnBrowseImagePath();
	void onBtnBrowseOutPath();
	void onAkazeCheck();
	void onSiftCheck();
	void onAkaze_SiftCheck();
	void onMvsCheck();
	void onSMvsCheck();
	void onOpenModelClick();
	void onReconFinished();
	void onSeperateRecon();
	void setProgress(int total, int sub, char* msg);
	void onDefaultCheck();
	void onMakeSceneBtn();
	void onSfmReconBtn();
	void onDmReconBtn();
	void onFssReconBtn();
	void onTexReconBtn();
	void onScalingTexMesh();
};

