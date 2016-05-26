#pragma once

#include <tins/pdu.h>

#define DMP_ETH_TYPE 0x8ae

namespace Tins {
	class DMP : public PDU {
	public:
	    /* 
	     * Unique protocol identifier. For user-defined PDUs, you **must**
	     * use values greater or equal to PDU::USER_DEFINED_PDU;
	     */
	    static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

	    /*
	     * Constructor from buffer. This constructor will be called while
	     * sniffing packets, whenever a PDU of this type is found. 
	     * 
	     * The "data" parameter points to a buffer of length "sz". 
	     */
	    DMP(const uint8_t* data, uint32_t sz) : buffer(data, data + sz) { }

	    DMP() {}
	    
	    /*
	     * Clones the PDU. This method is used when copying PDUs.
	     */
	    DMP *clone() const { return new DMP(*this); }
	    
	    /*
	     * Retrieves the size of this PDU. 
	     */
	    uint32_t header_size() const { return buffer.size(); }
	    
	    /*
	     * This method must return pdu_flag.
	     */
	    PDUType pdu_type() const { return pdu_flag; }
	    
	    /*
	     * Serializes the PDU. The serialization output should be written
	     * to the buffer pointed to by "data", which is of size "sz". The
	     * "sz" parameter will be equal to the value returned by 
	     * DummyPDU::header_size. 
	     * 
	     * The third parameter is a pointer to the parent PDU. You shouldn't
	     * normally need to use this. 
	     */
	    void write_serialization(uint8_t *data, uint32_t sz, const PDU *parent) 
	    { 
	        std::copy(buffer.begin(), buffer.end(), data);
	    }
	    
	    // This is just a getter to retrieve the buffer member.
	    const std::vector<uint8_t> &get_buffer() const 
	    {
	        return buffer;
	    }
	private:
	    std::vector<uint8_t> buffer;
	};
}