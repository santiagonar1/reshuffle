#ifndef AUTOPAS_PARTICLE_HPP
#define AUTOPAS_PARTICLE_HPP

#include <array>
#include <iosfwd>
#include <tuple>
#include <zpp_bits.h>

enum class OwnershipState : int64_t {
    dummy = 0b0000,// 0
    owned = 0b0001,// 1
    halo = 0b0010, // 2
};

template<typename floatType, typename idType>
class ParticleBase {
public:
    ParticleBase()
        : _r({0.0, 0.0, 0.0}), _v({0., 0., 0.}), _f({0.0, 0.0, 0.0}), _id(0),
          _ownershipState(OwnershipState::owned), _rAtRebuild({0.0, 0.0, 0.0}) {}

    /**
   * Constructor of the Particle class.
   * @param r Position of the particle.
   * @param v Velocity of the particle.
   * @param id Id of the particle.
   * @param ownershipState OwnershipState of the particle (can be either owned, halo, or dummy)
   */
    ParticleBase(const std::array<double, 3> &r, const std::array<double, 3> &v, idType id,
                 OwnershipState ownershipState = OwnershipState::owned)
        : _r(r), _v(v), _f({0.0, 0.0, 0.0}), _id(id), _ownershipState(ownershipState),
          _rAtRebuild(r) {}

    /**
   * Destructor of ParticleBase class
   */
    virtual ~ParticleBase() = default;

protected:
    std::array<floatType, 3> _r;
    std::array<floatType, 3> _rAtRebuild;
    std::array<floatType, 3> _v;
    std::array<floatType, 3> _f;
    idType _id;
    OwnershipState _ownershipState;

public:
    template<typename T, typename P>
    friend std::ostream &operator<<(std::ostream &os, const ParticleBase<T, P> &D);


    bool operator==(const ParticleBase &rhs) const {
        return std::tie(_r, _v, _f, _id) == std::tie(rhs._r, rhs._v, rhs._f, rhs._id);
    }

    bool operator!=(const ParticleBase &rhs) const { return not(rhs == *this); }
};

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::array<T, 3> &array) {
    os << "{ " << array[0] << ", " << array[1] << ", " << array[2] << " }";
    return os;
}

template<typename floatType, typename idType>
std::ostream &operator<<(std::ostream &os, const ParticleBase<floatType, idType> &particle) {
    os << "Particle"
       << "\nID      : " << particle._id << "\nPosition: " << particle._r
       << "\nVelocity: " << particle._v << "\nForce   : " << particle._f;
    // clang-format on
    return os;
}

using ParticleBaseFP64 = ParticleBase<double, unsigned long>;

class MoleculeLJ final : public ParticleBaseFP64 {
public:
    static constexpr auto serialize(auto &archive, MoleculeLJ &m) {
        return archive(m._typeId, m._oldF, m._r, m._v, m._f, m._id, m._ownershipState,
                       m._rAtRebuild);
    }

    static constexpr auto serialize(auto &archive, const MoleculeLJ &m) {
        return archive(m._typeId, m._oldF, m._r, m._v, m._f, m._id, m._ownershipState,
                       m._rAtRebuild);
    }

    MoleculeLJ() = default;

    /**
   * Constructor of lennard jones molecule with initialization of typeID.
   * @param pos Position of the molecule.
   * @param v Velocity of the molecule.
   * @param moleculeId Unique Id of the molecule.
   * @param typeId TypeId of the molecule.
   */
    MoleculeLJ(const std::array<double, 3> &pos, const std::array<double, 3> &v,
               unsigned long moleculeId, unsigned long typeId = 0)
        : ParticleBaseFP64(pos, v, moleculeId), _typeId(typeId) {}

    ~MoleculeLJ() override = default;


    bool operator==(const MoleculeLJ &rhs) const {
        return std::tie(this->_r, this->_v, this->_f, this->_id, _typeId, _oldF) ==
               std::tie(rhs._r, rhs._v, rhs._f, rhs._id, rhs._typeId, rhs._oldF);
    }

    bool operator!=(const MoleculeLJ &rhs) const { return not(rhs == *this); }


protected:
    size_t _typeId = 0;
    std::array<double, 3> _oldF = {0., 0., 0.};
};

RESHUFFLE_HAS_FIXED_SIZE_SERIALIZABLE(MoleculeLJ)

#endif//AUTOPAS_PARTICLE_HPP
