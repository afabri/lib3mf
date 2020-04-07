#include "Model/Classes/NMR_KeyStoreResourceData.h"
#include "Common/NMR_Exception.h"
#include <memory>

#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32

namespace NMR {


	nfUint64 CKeyStoreResourceData::s_nfHandleCount = 0;

	void CKeyStoreResourceData::initializeCipher() {
		m_sCipherValue.m_iv.resize(IV_SIZE, 0);
		m_sCipherValue.m_tag.resize(TAG_SIZE, 0);
		m_sCipherValue.m_key.resize(KEY_SIZE, 0);
		initializeKey();
		initializeIV();
	}

	void CKeyStoreResourceData::initializeKey() {
		nfUint64 rc = m_fnRandCall(m_sCipherValue.m_key.data(), (int)m_sCipherValue.m_key.size());
		if (rc != 1)
			throw CNMRException(NMR_ERROR_CALCULATIONTERMINATED);
	}

	void CKeyStoreResourceData::initializeIV() {
		nfUint64 rc = m_fnRandCall(m_sCipherValue.m_iv.data(), (int)m_sCipherValue.m_iv.size());
		if (rc != 1)
			throw CNMRException(NMR_ERROR_CALCULATIONTERMINATED);
	}

	CKeyStoreResourceData::CKeyStoreResourceData(std::string const & path, CryptoRandCbType & randCall) {
		if (randCall)
			m_fnRandCall = randCall;
		else {
			//TODO: need to use internal call
			throw CNMRException(NMR_ERROR_NOTIMPLEMENTED);
		}
		m_sPath = path;
		m_EncryptionAlgorithm = eKeyStoreEncryptAlgorithm::Aes256Gcm;
		m_nfHandle = ++s_nfHandleCount;
		initializeCipher();
	}

	CKeyStoreResourceData::CKeyStoreResourceData(std::string const & path, eKeyStoreEncryptAlgorithm const & ea, nfBool const & compression, CryptoRandCbType & randCall)
	{
		if (randCall)
			m_fnRandCall = randCall;
		else {
			//TODO: need to use internal call
			throw CNMRException(NMR_ERROR_NOTIMPLEMENTED);
		}
		m_sPath = path;
		m_EncryptionAlgorithm = ea;
		m_bCompression = compression;
		m_nfHandle = ++s_nfHandleCount;
		initializeCipher();
	}

	PKeyStoreDecryptRight CKeyStoreResourceData::addDecryptRight(NMR::PKeyStoreConsumer const& consumer, eKeyStoreEncryptAlgorithm const& encryptAlgorithm) {
		PKeyStoreDecryptRight dr = std::make_shared<CKeyStoreDecryptRight>(consumer, encryptAlgorithm);
		return addDecryptRight(dr);
	}

	PKeyStoreDecryptRight CKeyStoreResourceData::addDecryptRight(PKeyStoreDecryptRight const &dr)
	{	
		if (m_ConsumerDecryptRight.find(dr->getConsumer()->getConsumerID()) != m_ConsumerDecryptRight.end()) {
			throw CNMRException(NMR_ERROR_KEYSTOREDUPLICATEDECRYPTRIGHT);
		}

		m_DecryptRights.push_back(dr);
		m_ConsumerDecryptRight[dr->getConsumer()->getConsumerID()] = dr;
		return dr;
	}

	nfUint32 CKeyStoreResourceData::getDecryptRightCount()
	{	
		return (uint32_t)m_DecryptRights.size();
	}

	PKeyStoreDecryptRight CKeyStoreResourceData::getDecryptRight(nfUint32 index) const 
	{	
		if (index >= m_DecryptRights.size())
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		return m_DecryptRights[index];
	}

	PKeyStoreDecryptRight CKeyStoreResourceData::findDecryptRightByConsumer(NMR::PKeyStoreConsumer const& consumer) 
	{
		if (!consumer.get())
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (m_ConsumerDecryptRight.find(consumer->getConsumerID()) != m_ConsumerDecryptRight.end())
			return m_ConsumerDecryptRight[consumer->getConsumerID()];
		return nullptr;
	}

	void CKeyStoreResourceData::removeDecryptRight(NMR::PKeyStoreConsumer const& consumer)
	{
		if (!consumer.get())
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		size_t n = m_ConsumerDecryptRight.erase(consumer->getConsumerID());
		if (n > 0) {
			auto it = m_DecryptRights.begin();
			while (it != m_DecryptRights.end()) {
				if ((*it)->getConsumer()->getConsumerID() == consumer->getConsumerID()) {
					it = m_DecryptRights.erase(it);
				}
			}
		}
	}

	eKeyStoreEncryptAlgorithm CKeyStoreResourceData::getEncryptionAlgorithm() const
	{	
		return m_EncryptionAlgorithm;
	}

	nfBool CKeyStoreResourceData::getCompression() const
	{
		return m_bCompression;
	}
	NMR::PPackageModelPath CKeyStoreResourceData::getPath() const
	{
		NMR::CResourceHandler * pResourceHandler = new NMR::CResourceHandler();
		return pResourceHandler->makePackageModelPath(m_sPath);
	}

	nfUint64 CKeyStoreResourceData::getHandle() const {
		return m_nfHandle;
	}

	nfBool CKeyStoreResourceData::empty() const {
		return m_DecryptRights.empty();
	}

	CIPHERVALUE CKeyStoreResourceData::getCipherValue() const {
		return m_sCipherValue;
	}

	void CKeyStoreResourceData::setCipherValue(CIPHERVALUE const & cv) {
		m_sCipherValue = cv;
		m_bOpen = true;
	}

	bool CKeyStoreResourceData::isOpen() const {
		return m_bOpen;
	}

	void CKeyStoreResourceData::randomizeIV() {
		initializeIV();
	}

}
