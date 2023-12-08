#ifndef IDENTITY_SERVICE_GEN_H_
#define IDENTITY_SERVICE_GEN_H_ 1

#include <vector>
#include <mutex>
#include <map>
#include "identity-service.h"

class GenIdentityService: public IdentityService {
	private:
		NETID netid;
		KEY128 key;
        // helper data
        // helps to find out free address in the space
        uint32_t maxDevNwkAddr;
	protected:
		std::string masterKey;
		void clear();
        /**
          * Return next network address if available
          * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
          */
        int nextBruteForce(NETWORKIDENTITY &retVal);
	public:
		int errCode;
		std::string errMessage;

		GenIdentityService();
		GenIdentityService(const std::string &masterKey);
		~GenIdentityService() override;
		void setMasterKey(const std::string &masterKey);
		int get(DEVICEID &retval, const DEVADDR &devaddr) override;
		int getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui) override;
		// List entries
		int list(std::vector<NETWORKIDENTITY> &retVal, size_t offset, size_t size) override;
        // Entries count
        size_t size() override;
        int put(const DEVADDR &devaddr, const DEVICEID &id) override;
		int rm(const DEVADDR &addr) override;
		
		int init(const std::string &option, void *data) override;
		void flush() override;
		void done() override;
        /**
          * Return next network address if available
          * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
          */
        int next(NETWORKIDENTITY &retVal) override;
};

extern "C" IdentityService* makeGenIdentityService();

#endif
