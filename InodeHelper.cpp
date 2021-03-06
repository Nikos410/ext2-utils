#include "InodeHelper.hpp"

bool InodeHelper::is_directory() {
    // 0x4000
    return (inode_->i_mode >> 14) & 1;
}

bool InodeHelper::is_regularFile() {
    // 0x8000
    return (inode_->i_mode >> 15) & 1;
}

std::vector<char>& InodeHelper::copy_data() {
    data_.clear();

    for (int i = 0; i < 12; i++) {
        copy_data_block_if_neccessary(inode_->i_block[i]);
    }

    copy_indirect_block_if_neccessary(inode_->i_block[12]);
    copy_doubly_indirect_block_if_neccessary(inode_->i_block[13]);
    copy_trebly_indirect_block_if_neccessary(inode_->i_block[14]);

    return data_;
}

void InodeHelper::copy_data_block_if_neccessary(unsigned int block_number) {
    if (block_number == 0) {
        return;
    }

    char* data_block = block_helper_.get_block(block_number);
    for (int i = 0; i < block_helper_.get_block_size_in_bytes(); i++) {
        if (data_.size() >= inode_->i_size) {
            return;
        }

        data_.push_back(data_block[i]);
    }
}

void InodeHelper::copy_indirect_block_if_neccessary(unsigned int block_number) {
    char* indirect_block = block_helper_.get_block(block_number);
    unsigned int* direct_block_number = (unsigned int*) indirect_block;

    for (int i = 0; i < block_helper_.get_block_size_in_bytes() / sizeof(unsigned int); i++) {
        if (*direct_block_number == 0) {
            break;
        }
        copy_data_block_if_neccessary(*direct_block_number);
        direct_block_number++;
    }
}

void InodeHelper::copy_doubly_indirect_block_if_neccessary(unsigned int block_number) {
    char* doubly_indirect_block = block_helper_.get_block(block_number);
    unsigned int* indirect_block_number = (unsigned int*) doubly_indirect_block;

    for (int i = 0; i < block_helper_.get_block_size_in_bytes() / sizeof(unsigned int); i++) {
        if (*indirect_block_number == 0) {
            break;
        }
        copy_indirect_block_if_neccessary(*indirect_block_number);
        indirect_block_number++;
    }
}

void InodeHelper::copy_trebly_indirect_block_if_neccessary(unsigned int block_number) {
    char* trebly_indirect_block = block_helper_.get_block(block_number);
    unsigned int* doubly_indirect_block_number = (unsigned int*) trebly_indirect_block;

    for (int i = 0; i < block_helper_.get_block_size_in_bytes() / sizeof(unsigned int); i++) {
        if (*doubly_indirect_block_number == 0) {
            break;
        }
        copy_doubly_indirect_block_if_neccessary(*doubly_indirect_block_number);
        doubly_indirect_block_number++;
    }
}
