pushd %~dp0
:: ��Ȼ����ʧ�ܣ�ֻ��Ϊ�˼�������
net start SbieSvc
:: ����
richstuff-x86_64 -load_by_driver SbiePatch.sys
:: �������� sc start �� net start �������Ǻ���������������õ��������ɹ�
sc start SbieSvc
popd