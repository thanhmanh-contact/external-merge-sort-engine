# External Merge Sort – Memory Structure & I/O Mechanism

Project này cài đặt thuật toán **External Merge Sort** sử dụng kỹ thuật **Block-based I/O** và **Replacement Selection** để tối ưu hóa việc tạo các run ban đầu và giảm thiểu chi phí truy cập đĩa cứng.

---

## Mô hình bộ nhớ (Memory Model)

Thuật toán giả định hệ thống có các tham số cấu hình sau:

| Tham số | Ý nghĩa |
| :--- | :--- |
| **B** | Tổng số Buffer Page có trong RAM |
| **blockSize** | Kích thước của mỗi block (bytes) |
| **recordSize** | Kích thước 1 bản ghi (mặc định `double` = 8 bytes) |
| **recordsPerBlock** | Số lượng record trong 1 block ($blockSize / 8$) |

---

## Cấu trúc RAM theo giai đoạn

RAM được phân bổ linh hoạt tùy theo đặc thù của từng giai đoạn thuật toán.

### 1. Replacement Selection (Tạo run ban đầu)
Mục tiêu: Tạo ra các run có độ dài trung bình $\approx 2 \times$ kích thước heap.

**Phân bổ Buffer (Tổng B):**
* **1 Input Buffer:** Đọc dữ liệu từ Disk.
* **B - 2 Buffers:** Dành cho Heap & Frozen records.
* **1 Output Buffer:** Ghi dữ liệu xuống Disk.

> **Cơ chế Heap & Frozen:**
> * **Min-Heap:** Lưu trữ record thuộc run hiện tại. Kích thước tối đa: $heapCapacity = (B - 2) \times recordsPerBlock$.
> * **Frozen Records:** Nếu `newValue < lastOutput`, record đó bị "đóng băng" cho run tiếp theo.
> * **Quy tắc:** $\text{Heap} + \text{Frozen} \leq heapCapacity$.

---

### 2. Multi-way Merge (Hợp nhất Run)
Sau khi có các run, thuật toán thực hiện trộn chúng lại thành một file duy nhất đã sắp xếp.

**Phân bổ Buffer (Tổng B):**
* **B - 1 Input Buffers:** Mỗi buffer quản lý dữ liệu cho 1 run (Fan-in = $B-1$).
* **1 Output Buffer:** Chứa kết quả sau khi so sánh giữa các run.

**Quy trình Merge:**
1. Nạp block đầu tiên của mỗi run vào các Input Buffer.
2. Chọn record nhỏ nhất trong các buffer -> Đưa vào Output Buffer.
3. Nếu một Input Buffer trống: Đọc block tiếp theo từ run đó (`stats.addRead()`).
4. Nếu Output Buffer đầy: Ghi block xuống Disk (`stats.addWrite()`).

---

## Mô hình I/O và Thống kê
Mọi thao tác đọc/ghi đều thực hiện theo đơn vị **Block** để tối ưu tốc độ phần cứng.

* **Disk Read:** Thực hiện khi Input Buffer hết dữ liệu.
* **Disk Write:** Thực hiện khi Output Buffer đầy hoặc kết thúc một run.
* **Công thức chi phí:** $$\text{Total I/O} = \text{Disk Reads} + \text{Disk Writes}$$

---

## Demo Execution

Dưới đây là kết quả chạy thực tế với file dữ liệu mẫu:

![Execution Screenshot](/examples/output_screen.JPG)

**Phân tích kết quả:**
* **B = 5, BlockSize = 24**: Cấu hình bộ nhớ tối thiểu để kiểm tra tính đúng đắn.
* **Replacement Selection**: Tạo ra các Run có độ dài vượt trội so với kích thước Heap (9 records), giúp giảm số lượng Initial Runs xuống còn 3.
* **I/O Efficiency**: Tổng số lượt I/O thực tế (66) khớp với dự tính lý thuyết (~64), chứng minh cơ chế quản lý buffer hoạt động hiệu quả.

---

## Điểm mạnh của thiết kế
* **Tối ưu RAM:** Tận dụng tối đa $B$ buffer cho từng mục đích cụ thể.
* **Run dài hơn:** Replacement Selection giúp giảm số lượng run ban đầu, từ đó giảm số vòng merge.
* **Hiệu suất cao:** Sử dụng cấu trúc Min-Heap với độ phức tạp $O(\log n)$ để xử lý dữ liệu trong RAM.

---
*Dự án hỗ trợ sắp xếp các tệp dữ liệu lớn vượt quá dung lượng RAM thực tế.*