// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2017-2018 The VIRIDI Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <mutex>
#include <assert.h>
#include <limits>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of(0, uint256("00000b30721e977a9cd087fea593d1809c74386177afa76108b9f7b4eccc6e5e"));

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1506654183, // * UNIX timestamp of last checkpoint block
    50,      // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    2000        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of(0, uint256("000007cd8923b9abe8854afe9b1d5fee30d50d9a48c00ea108f69a59639656ad"));
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1506261240,
    0,
    250};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("46e179b98313838727265d83ffd60d50ea38219221160db357477ec073caa1ce"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1506262240,
    0,
    100};

const CChainParams::SubsidySwitchPoints& CChainParams::GetSubsidySwitchPoints(uint32_t nTime, int nHeight) const
{
    if(nTime <= nHEXHashTimestamp)
       return subsidySwitchPoints;
    else if(nTime <= nF2Timestamp)
       return subsidySwitchPoints_HEXHash;
    else if(nHeight < static_cast<int>(subsidyScheduleStart_F2))
        return subsidySwitchPoints_HEXHash;

    auto decrease_interval = std::min(subsidyDecreaseCount_F2, (nHeight - subsidyScheduleStart_F2) / subsidyDecreaseInterval_F2);

    return subsidySwitchPointsSchedule_F2.find(decrease_interval)->second;
}

CAmount CChainParams::SubsidyValue(SubsidySwitchPoints::key_type level, uint32_t nTime, int nHeight) const
{
    const auto& switch_points = GetSubsidySwitchPoints(nTime, nHeight);

    SubsidySwitchPoints::const_iterator point = switch_points.upper_bound(level);

    if(point != switch_points.begin())
        point = std::prev(point);

    return point->second;
}

void CChainParams::initSubsidySwitchPointsSchedule()
{
    subsidySwitchPointsSchedule_F2[0u] = subsidySwitchPoints_F2_0;

    for(auto i = 1u; i <= subsidyDecreaseCount_F2; ++i)
    {
       subsidySwitchPointsSchedule_F2[i] = subsidySwitchPointsSchedule_F2[i - 1];

       for(auto& sp : subsidySwitchPointsSchedule_F2[i])
       {
           auto prev_value = sp.second;

           sp.second *= 10000u - subsidyDecreaseValue_F2;
           sp.second /= 10000u;
           sp.second += COIN / 10u - 1u;
           sp.second /= COIN / 10u;
           sp.second *= COIN / 10u;

           if(sp.second == prev_value && sp.second > COIN / 10u)
             sp.second -= COIN / 10u;
       }
    }
}

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0x20;
        pchMessageStart[1] = 0xe1;
        pchMessageStart[2] = 0xcf;
        pchMessageStart[3] = 0x18;
        vAlertPubKey = ParseHex("04A2B684CBABE97BA08A35EA388B06A6B03E13DFBA974466880AF4CAE1C5B606A751BF7C5CBDE5AB90722CF5B1EC1AADA6D24D607870B6D6B5D684082655404C8D");
        vVIRIDIDevKey = ParseHex("0204e1ae7133cedcf8ecff227d0371e27e6f8f11771630f2c2dfae8d0d390ec80d"); // DevPubKey for fees
        vVIRIDIFundKey = ParseHex("02dd68d9078238d04aef3d31b3aa29b1dc148097ac081b85327ee14b76143da572"); // FundPubKey for fees
        nDevFee = 1; // DevFee %
        nFundFee = 1; //FundFee %
        nDefaultPort = 2706;
        bnProofOfWorkLimit = ~uint256(0) >> 20;
        bnStartWork = ~uint256(0) >> 24;

        subsidySwitchPoints = {
            {0 , 5 * COIN},
            
        };
        assert(subsidySwitchPoints.size());

        subsidySwitchPoints_HEXHash = {
            {0 , 5 * COIN},
        };
        assert(subsidySwitchPoints_HEXHash.size());

        subsidySwitchPoints_F2_0 = {
            {0         ,   38  * (COIN/10)},
            {20   * 1e9,   47  * (COIN/10)},
            {30   * 1e9,   66  * (COIN/10)},
            {50   * 1e9,   94  * (COIN/10)},
            {80   * 1e9,  131  * (COIN/10)},
            {130  * 1e9,  177  * (COIN/10)},
            {210  * 1e9,  233  * (COIN/10)},
            {340  * 1e9,  298  * (COIN/10)},
            {550  * 1e9,  373  * (COIN/10)},
            {890  * 1e9,  456  * (COIN/10)},
            {1440 * 1e9,  550  * (COIN/10)},
            {2330 * 1e9,  652  * (COIN/10)},
            {3770 * 1e9,  764  * (COIN/10)},
            {6100 * 1e9,  885  * (COIN/10)},
            {9870 * 1e9,  1015 * (COIN/10)},
        };
        assert(subsidySwitchPoints_F2_0.size());

        subsidyScheduleStart_F2    = 750; // block#XXXXXX ~= nF2Timestamp + 1 day
        subsidyDecreaseInterval_F2 = 43200;  // 43200 bloks ~= 30 days
        subsidyDecreaseCount_F2    = 23;     // 23
        subsidyDecreaseValue_F2    = 694;    // 694 = 6,94% * 100

        initSubsidySwitchPointsSchedule();

        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
        nTargetSpacing = 1 * 60;  // VIRIDI: 1 minute
        nAntiInstamineTime = 100; // 100 blocks with 1 reward for instamine prevention
        nMaturity = 60;
        nMasternodeCountDrift = 3;
        nMaxMoneyOut = 200000000 * COIN;

        nStartMasternodePaymentsBlock = 120;

        /** Height or Time Based Activations **/
        nLastPOWBlock = 1440000;
        nModifierUpdateBlock = std::numeric_limits<decltype(nModifierUpdateBlock)>::max();

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
         *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
         *   vMerkleTree: e0028e
         */
        const char* pszTimestamp = "The born of Viridi - 2017-09-29 03:03:03";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 50 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("044a001040da79684a0544c2254eb6c896fae95a9ea7b51d889475eb57ab2051f1a5858cac61ae400e90ea08015263ad40c65d36f0edf19e996972e7d2cbd13c15") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime = 1506654183;
        genesis.nBits = 0x1e0ffff0;
        genesis.nNonce = 960862;

        hashGenesisBlock = genesis.GetKeccakHash();

        assert(hashGenesisBlock == uint256("00000b30721e977a9cd087fea593d1809c74386177afa76108b9f7b4eccc6e5e"));
        assert(genesis.hashMerkleRoot == uint256("6b6c68db10692dc2d4c6685c7f6ffca1e07aaf3b802cef9b0459a34c626c1190"));

        vSeeds.push_back(CDNSSeedData("viridicoin.net", "seed.viridicoin.net"));     // Primary DNS Seeder

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 70);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 8);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 212);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x02)(0x2D)(0x25)(0x33).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0x21)(0x31)(0x2B).convert_to_container<std::vector<unsigned char> >();
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x07)(0x99).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        strSporkKey = "049825D3D5FC2AF3AAEB0E7AEF080FD52B0ABDD68B8519A32A917BB764409A971BD33EC3FADA75E43EDED6F14D8F7D5A9A2B94F1BB08045D2499EE23732937A902";
        strObfuscationPoolDummyAddress = "VTQF6gfnV77jnxXxkauDxPqkW3nmMEkiZ1";
        nStartMasternodePayments = 1403728576; //Wed, 25 Jun 2014 20:36:16 GMT

        nHEXHashTimestamp = 1554465600;  // Friday, 5 April 2019 12:00:00 GMT+00:00
        nF2Timestamp      = 1870020488; // Wednesday, 4 April 2029 18:08:08 GMT+00:00
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x30;
        pchMessageStart[1] = 0xf1;
        pchMessageStart[2] = 0xcc;
        pchMessageStart[3] = 0x28;

        bnProofOfWorkLimit = ~uint256(0) >> 1;
        bnStartWork = bnProofOfWorkLimit;

        subsidySwitchPoints = {
           {0         ,   1 * COIN},
        };
        assert(subsidySwitchPoints.size());

        vAlertPubKey = ParseHex("04459DC949A9E2C2E1FA87ED9EE93F8D26CD52F95853EE24BCD4B07D4B7D79458E81F0425D81E52B797ED304A836667A1D2D422CD10F485B06CCBE906E1081FBAC");
        nDefaultPort = 12706;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetSpacing = 1 * 60;  // VIRIDI: 1 minute
        nLastPOWBlock = std::numeric_limits<decltype(nLastPOWBlock)>::max();
        nMaturity = 15;
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = std::numeric_limits<decltype(nModifierUpdateBlock)>::max();
        nMaxMoneyOut = 200000000 * COIN;


        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1506261240;
        genesis.nNonce = 1396025;

        hashGenesisBlock = genesis.GetKeccakHash();

        assert(hashGenesisBlock == uint256("000007cd8923b9abe8854afe9b1d5fee30d50d9a48c00ea108f69a59639656ad"));

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("viridi.io", "seed01.viridi.io"));     // Primary DNS Seeder

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 132); // Testnet VIRIDI addresses start with 'v'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet VIRIDI script addresses start with '8' or '9'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet VIRIDI BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet VIRIDI BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet VIRIDI BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        strSporkKey = "0421838CC1407E7B8C0C5F2379DF7EBD395181949CFA55124939B4980D5054A7926F88E3059921A50F0F81C5195E882D9A414EA0835BB89C9BB061511B9F132B31";
        strObfuscationPoolDummyAddress = "VPYhw53Z8dPasuHdSKeuByo8QfhfdkRWAH";
        nStartMasternodePayments = 1420837558; //Fri, 09 Jan 2015 21:05:58 GMT
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0x31;
        pchMessageStart[1] = 0xf1;
        pchMessageStart[2] = 0xcc;
        pchMessageStart[3] = 0x21;

        bnStartWork = ~uint256(0) >> 20;

        subsidySwitchPoints = {
           {0         ,   1 * COIN},
        };
        assert(subsidySwitchPoints.size());

        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetSpacing = 1 * 60;        // VIRIDI: 1 minute
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        genesis.nTime = 1506262240;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 1;

        hashGenesisBlock = genesis.GetKeccakHash();
        nDefaultPort = 51476;

        assert(hashGenesisBlock == uint256("46e179b98313838727265d83ffd60d50ea38219221160db357477ec073caa1ce"));

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 32706;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fMineBlocksOnDemand = true;

        subsidySwitchPoints = {
            {0, 1 * COIN},
        };
        assert(subsidySwitchPoints.size());

    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
