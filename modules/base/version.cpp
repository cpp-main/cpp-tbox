namespace tbox {

void GetTboxVersion(int &major, int &minor, int &rev)
{
    major = TBOX_VERSION_MAJOR;
    minor = TBOX_VERSION_MINOR;
    rev   = TBOX_VERSION_REVISION;
}

}
