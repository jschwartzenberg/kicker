/*
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "soliddeviceengine.h"

#include <KDebug>
#include <KLocale>

#include "plasma/datasource.h"

SolidDeviceEngine::SolidDeviceEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
	Q_UNUSED(args)
	devicelist = QStringList();
	
	//used to reference device type
	typelist = QStringList();
	typelist << "Unknown" << "GenericInterface" << "Processor"
			<< "Block" << "StorageAccess" << "StorageDrive"
			<< "OpticalDrive" << "StorageVolume" << "OpticalDisc"
			<< "Camera" << "PortableMediaPlayer" << "NetworkInterface"
			<< "AcAdapter" << "Battery" << "Button" << "AudioInterface"
			<< "DvbInterface";
}

SolidDeviceEngine::~SolidDeviceEngine()
{
}

bool SolidDeviceEngine::sourceRequested(const QString &name)
{
	/* This creates a list of all available devices.  This must be called first before any other sources
	 * will be available.
	 */
	if(name == i18n("Devices"))
	{
		//if the devicelist is already populated, return
		if(!devicelist.isEmpty()){ return true; }
		
		//get list of devices already detected
		QList<Solid::Device> devices =  Solid::Device::allDevices();
		foreach(Solid::Device device, devices)
		{
			devicelist << device.udi();
		}
		kDebug() << "Device list:" << devicelist << endl;
		if(devicelist.isEmpty() ){ return false; }
		setData(name, "DeviceList", devicelist);
		
		//detect when new devices are added
		Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();
		connect(notifier, SIGNAL(deviceAdded(const QString&)),
				this, SLOT(deviceAdded(const QString&)));
		connect(notifier, SIGNAL(deviceRemoved(const QString&)),
				this, SLOT(deviceRemoved(const QString&)));
		
		return true;
	}
	else
	{
		if(devicelist.contains(name) )
		{
			kDebug() << "Requesting " << name << endl;
			return populateDeviceData(name);
		}
		else {return false; }
	}
}

bool SolidDeviceEngine::populateDeviceData(const QString &name)
{
	Solid::Device device(name);
	if(!device.isValid()) { return false; }
	
	QStringList devicetypes = QStringList();
	setData(name, "ParentUDI", device.parentUdi());
	setData(name, "Vendor", device.vendor());
	setData(name, "Product", device.product());
	setData(name, "Icon", device.icon());
	
	if(device.is<Solid::Processor>())
	{
		Solid::Processor *processor = device.as<Solid::Processor>();
		if(processor == 0){ return false; }
		
		devicetypes << "Processor";
		setData(name, "Number", processor->number());
		setData(name, "MaxSpeed", processor->maxSpeed());
		setData(name, "CanChangeFrequency", processor->canChangeFrequency());
	}
	if(device.is<Solid::Block>())
	{
		Solid::Block *block = device.as<Solid::Block>();
		if(block == 0){ return false; }
				
		devicetypes << "Block";
		setData(name, "Major", block->deviceMajor());
		setData(name, "Minor", block->deviceMajor());
		setData(name, "Device", block->device());
	}
	if(device.is<Solid::StorageAccess>())
	{
		Solid::StorageAccess *storageaccess = device.as<Solid::StorageAccess>();
		if(storageaccess == 0) return false;
				
		devicetypes << "StorageAccess";
		setData(name, "Accessible",storageaccess->isAccessible());
		setData(name, "FilePath", storageaccess->filePath());
	}
	if(device.is<Solid::StorageDrive>())
	{
		Solid::StorageDrive *storagedrive = device.as<Solid::StorageDrive>();
		if(storagedrive == 0){ return false; }
				
		devicetypes << "StorageDrive";
		
		QStringList bus = QStringList();
		bus << "Ide" <<	"Usb" << "Ieee1394" << "Scsi" << "Sata" << "Platform";
		QStringList drivetype = QStringList();
		drivetype << "HardDisk" <<  "CdromDrive" <<  "Floppy" <<  "Tape" <<  "CompactFlash" <<  "MemoryStick" <<  "SmartMedia" <<  "SdMmc" <<  "Xd";
				
		setData(name, "Bus", bus.at((int)storagedrive->bus()));
		setData(name, "DriveType", drivetype.at((int)storagedrive->driveType()));
		setData(name, "Removable", storagedrive->isRemovable());
		setData(name, "EjectRequired", storagedrive->isEjectRequired());
		setData(name, "Hotpluggable", storagedrive->isHotpluggable());
		setData(name, "MediaCheckEnabled", storagedrive->isMediaCheckEnabled());
		setData(name, "Vendor", storagedrive->vendor());
		setData(name, "Product", storagedrive->product());
	}
	if(device.is<Solid::OpticalDrive>())
	{
		Solid::OpticalDrive *opticaldrive = device.as<Solid::OpticalDrive>();
		if(opticaldrive == 0){ return false; }
				
		devicetypes << "OpticalDrive";
		
		QStringList supportedtypes = QStringList();
		Solid::OpticalDrive::MediumTypes mediatypes = opticaldrive->supportedMedia();
		if(mediatypes & Solid::OpticalDrive::Cdr){ supportedtypes << "Cdr"; }
		if(mediatypes & Solid::OpticalDrive::Cdrw){ supportedtypes << "Cdrw"; }
		if(mediatypes & Solid::OpticalDrive::Dvd){ supportedtypes << "Dvd"; }
		if(mediatypes & Solid::OpticalDrive::Dvdr){ supportedtypes << "Dvdr"; }
		if(mediatypes & Solid::OpticalDrive::Dvdrw){ supportedtypes << "Dvdrw"; }
		if(mediatypes & Solid::OpticalDrive::Dvdram){ supportedtypes << "Dvdram"; }
		if(mediatypes & Solid::OpticalDrive::Dvdplusr){ supportedtypes << "Dvdplusr"; }
		if(mediatypes & Solid::OpticalDrive::Dvdplusrw){ supportedtypes << "Dvdplusrw"; }
		if(mediatypes & Solid::OpticalDrive::Dvdplusdl){ supportedtypes << "Dvdplusdl"; }
		if(mediatypes & Solid::OpticalDrive::Dvdplusdlrw){ supportedtypes << "Dvdplusdlrw"; }
		if(mediatypes & Solid::OpticalDrive::Bd){ supportedtypes << "Bd"; }
		if(mediatypes & Solid::OpticalDrive::Bdr){ supportedtypes << "Bdr"; }
		if(mediatypes & Solid::OpticalDrive::Bdre){ supportedtypes << "Bdre"; }
		if(mediatypes & Solid::OpticalDrive::HdDvd){ supportedtypes << "HdDvd"; }
		if(mediatypes & Solid::OpticalDrive::HdDvdr){ supportedtypes << "HdDvdr"; }
		if(mediatypes & Solid::OpticalDrive::HdDvdrw){ supportedtypes << "HdDvdrw"; }
		setData(name, "SupportedMedia", supportedtypes);
				
		setData(name, "ReadSpeed", opticaldrive->readSpeed());
		setData(name, "WriteSpeed", opticaldrive->writeSpeed());
		
		//the following method return QList<int> so we need to convert it to QList<QVariant>
		QList<int> writespeeds = opticaldrive->writeSpeeds();
		QList<QVariant> variantlist = QList<QVariant>();
		foreach(int num, writespeeds)
		{
			variantlist << QVariant(num);
		}
		setData(name, "WriteSpeeds", variantlist);
		
	}
	if(device.is<Solid::StorageVolume>())
	{
		Solid::StorageVolume *storagevolume = device.as<Solid::StorageVolume>();
		if(storagevolume == 0){ return false; }
				
		devicetypes << "StorageVolume";
		
		QStringList usagetypes = QStringList();
		usagetypes << "FileSystem" << "PartitionTable" << "Raid" << "Other" << "Unused";
				
		setData(name, "Ignored", storagevolume->isIgnored());
		setData(name, "Usage", usagetypes.at((int)storagevolume->usage()));
		setData(name, "FileSystemType", storagevolume->fsType());
		setData(name, "Label", storagevolume->label());
		setData(name, "Uuid", storagevolume->uuid());
		setData(name, "Size", storagevolume->size());
	}
	if(device.is<Solid::OpticalDisc>())
	{
		Solid::OpticalDisc *opticaldisc = device.as<Solid::OpticalDisc>();
		if(opticaldisc == 0){ return false; }
		
		devicetypes << "OpticalDisc";
		
		//get the content types
		QStringList contenttypelist = QStringList();
		Solid::OpticalDisc::ContentTypes contenttypes = opticaldisc->availableContent();
		if(contenttypes & Solid::OpticalDisc::Audio){ contenttypelist << "Audio"; }
		if(contenttypes & Solid::OpticalDisc::Audio){ contenttypelist << "Data"; }
		if(contenttypes & Solid::OpticalDisc::Audio){ contenttypelist << "VideoCd"; }
		if(contenttypes & Solid::OpticalDisc::Audio){ contenttypelist << "SuperVideoCd"; }
		if(contenttypes & Solid::OpticalDisc::Audio){ contenttypelist << "VideoDvd"; }
		setData(name, "AvailableContent", contenttypelist);
				
		QStringList disctypes = QStringList();
		disctypes << "UnknownDiscType" << "CdRom" << "CdRecordable"
				<< "CdRewritable" << "DvdRom" << "DvdRam" 
				<< "DvdRecordable" << "DvdRewritable" << "DvdPlusRecordable"
				<< "DvdPlusRewritable" << "DvdPlusRecordableDuallayer" << "DvdPlusRewritableDuallayer"
				<< "BluRayRom" << "BluRayRecordable" << "BluRayRewritable"
				<< "HdDvdRom" <<  "HdDvdRecordable" << "HdDvdRewritable";
				//+1 because the enum starts at -1
		setData(name, "DiscType", disctypes.at((int)opticaldisc->discType()+1));
		setData(name, "Appendable", opticaldisc->isAppendable());
		setData(name, "Blank", opticaldisc->isBlank());
		setData(name, "Rewritable", opticaldisc->isRewritable());
		setData(name, "Capacity", opticaldisc->capacity());
	}
	if(device.is<Solid::Camera>())
	{
		Solid::Camera *camera = device.as<Solid::Camera>();
		if(camera == 0){ return false; }
				
		devicetypes << "Camera";
		
		setData(name, "SupportedProtocols", camera->supportedProtocols());
		setData(name, "SupportedDrivers", camera->supportedDrivers());
	}
	if(device.is<Solid::PortableMediaPlayer>())
	{
		Solid::PortableMediaPlayer *mediaplayer = device.as<Solid::PortableMediaPlayer>();
		if(mediaplayer == 0){ return false; }
				
		devicetypes << "PortableMediaPlayer";
		
		setData(name, "SupportedProtocols", mediaplayer->supportedProtocols());
		setData(name, "SupportedDrivers", mediaplayer->supportedDrivers());
	}
	if(device.is<Solid::NetworkInterface>())
	{
		Solid::NetworkInterface *networkinterface = device.as<Solid::NetworkInterface>();
		if(networkinterface == 0){ return false;}
				
		devicetypes << "NetworkInterface";
		
		setData(name, "IfaceName", networkinterface->ifaceName());
		setData(name, "Wireless", networkinterface->isWireless());
		setData(name, "HwAddress", networkinterface->hwAddress());
		setData(name, "MacAddress", networkinterface->macAddress());
	}
	if(device.is<Solid::AcAdapter>())
	{
		Solid::AcAdapter *ac = device.as<Solid::AcAdapter>();
		if(ac == 0){ return false; }
				
		devicetypes << "AcAdapter";
		
		setData(name, "Plugged", ac->isPlugged());
	}
	if(device.is<Solid::Battery>())
	{
		Solid::Battery *battery = device.as<Solid::Battery>();
		if(battery == 0){ return false; }
				
		devicetypes << "Battery";
		
		QStringList batterytype = QStringList();
		batterytype << "UnknownBattery" << "PdaBattery" << "UpsBattery"
				<< "PrimaryBattery" << "MouseBattery" << "KeyboardBattery"
				<< " KeyboardMouseBattery" << "CameraBattery";
				
		QStringList chargestate = QStringList();
		chargestate << "NoCharge" << "Charging" << "Discharging";
				
		setData(name, "Plugged", battery->isPlugged());
		setData(name, "Type", batterytype.at((int)battery->type()));
		setData(name, "ChargeValueUnit", battery->chargeValueUnit());
		setData(name, "ChargeValue", battery->chargeValue());
		setData(name, "ChargePercent", battery->chargePercent());
		setData(name, "VoltageUnit", battery->voltageUnit());
		setData(name, "Voltage", battery->voltage());
		setData(name, "Rechargeable", battery->isRechargeable());
		setData(name, "ChargeState", chargestate.at((int)battery->chargeState()));
	}
	if(device.is<Solid::Button>())
	{
		Solid::Button *button = device.as<Solid::Button>();
		if(button == 0){ return false; }
				
		devicetypes << "Button";
		
		QStringList buttontype = QStringList();
		buttontype << "LidButton" << "PowerButton" << "SleepButton" << "UnknownButtonType";
				
		setData(name, "Type", buttontype.at((int)button->type()));
		setData(name, "HasState", button->hasState());
		setData(name, "StateValue", button->stateValue());
	}
	if(device.is<Solid::AudioInterface>())
	{
		Solid::AudioInterface *audiointerface = device.as<Solid::AudioInterface>();
		if(audiointerface == 0){ return false; }
				
		devicetypes << "AudioInterface";
		
		QStringList audiodriver = QStringList();
		audiodriver << "Alsa" << "OpenSoundSystem" << "UnknownAudioDriver";
				
		setData(name, "Driver", audiodriver.at((int)audiointerface->driver()));
		setData(name, "DriverHandles", audiointerface->driverHandles());
		setData(name, "Name", audiointerface->name());
				
				//get AudioInterfaceTypes
		QStringList audiointerfacetypes = QStringList();
		Solid::AudioInterface::AudioInterfaceTypes devicetypes = audiointerface->deviceType();
		if(devicetypes & Solid::AudioInterface::UnknownAudioInterfaceType){ audiointerfacetypes << "UnknownAudioInterfaceType";}
		if(devicetypes & Solid::AudioInterface::AudioControl){ audiointerfacetypes << "AudioControl"; }
		if(devicetypes & Solid::AudioInterface::AudioInput){ audiointerfacetypes << "AudioInput"; }
		if(devicetypes & Solid::AudioInterface::AudioOutput){audiointerfacetypes << "AudioOutput"; }
		setData(name, "DeviceType", audiointerfacetypes);
				
				//get SoundCardTypes
		QStringList soundcardtype = QStringList();
		soundcardtype << "InternalSoundcard" << "UsbSoundcard" << "FirewireSoundcard" << "Headset" << "Modem";
		setData(name, "SoundcardType", soundcardtype.at((int)audiointerface->soundcardType()));
	}
	if(device.is<Solid::DvbInterface>())
	{
		Solid::DvbInterface *dvbinterface = device.as<Solid::DvbInterface>();
		if(dvbinterface == 0){ return false; }
				
		devicetypes << "DvbInterface";
		
		setData(name, "Device", dvbinterface->device());
		setData(name, "DeviceAdapter", dvbinterface->deviceAdapter());
				
		//get devicetypes
		QStringList dvbdevicetypes = QStringList();
		dvbdevicetypes << "DvbUnknown" << "DvbAudio" << "DvbCa"
				<< "DvbDemux" << "DvbDvr" << "DvbFrontend"
				<< "DvbNet" << "DvbOsd" << "DvbSec" << "DvbVideo";
				
		setData(name, "DvbDeviceType", dvbdevicetypes.at((int)dvbinterface->deviceType()));
		setData(name, "DeviceIndex", dvbinterface->deviceIndex());
	}
	setData(name, "DeviceTypes", devicetypes);
	return true;
}

void SolidDeviceEngine::deviceAdded(const QString& udi)
{
	devicelist << udi;
	setData(i18n("Devices"), "DeviceList", devicelist);

	checkForUpdates();
}

void SolidDeviceEngine::deviceRemoved(const QString& udi)
{
	int pos = devicelist.indexOf(udi);
	if(pos > -1)
	{
		devicelist.removeAt(pos);
		removeSource(udi);
		setData(i18n("Devices"), "DeviceList", devicelist);
	}
	checkForUpdates();
}

#include "soliddeviceengine.moc"