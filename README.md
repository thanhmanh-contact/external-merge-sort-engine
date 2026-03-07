External Merge Sort – Cấu trúc RAM và cơ chế I/O

Project này cài đặt External Merge Sort sử dụng block-based I/O và Replacement Selection để tạo các run ban đầu.

Các record trong file là số thực double (8 bytes) và việc đọc/ghi disk được thực hiện theo block.

Mô hình bộ nhớ (Memory Model)

Thuật toán giả định rằng trong RAM có:

B buffer page.

Mỗi buffer page có kích thước:

blockSize bytes

Mỗi record có kích thước:

sizeof(double) = 8 bytes

Do đó số record trong một block là:

recordsPerBlock = blockSize / 8

Cấu trúc RAM

RAM được chia khác nhau tùy theo giai đoạn của thuật toán.

1. Replacement Selection (Tạo run ban đầu)

Trong giai đoạn tạo run ban đầu, RAM được chia như sau:

1 buffer input
B-2 buffer heap + frozen
1 buffer output

Tổng cộng:

B buffer

Input Buffer

Input buffer dùng để đọc block từ disk.

Luồng dữ liệu:

Disk → Input Buffer

Khi input buffer đọc hết dữ liệu, block tiếp theo sẽ được đọc từ disk.

Heap (Min Heap)

Heap lưu các record có thể thuộc run hiện tại.

Kích thước tối đa của heap:

heapCapacity = (B - 2) × recordsPerBlock

Heap được tổ chức dưới dạng min heap để luôn lấy được record nhỏ nhất nhanh nhất.

Hai thao tác chính:

pop_heap → lấy record nhỏ nhất
push_heap → thêm record mới

Độ phức tạp:

O(log n)

Frozen Records

Nếu record mới đọc từ input nhỏ hơn record vừa ghi ra output, record đó không thể nằm trong run hiện tại.

Khi đó record sẽ được frozen và dùng cho run tiếp theo.

Quy tắc:

if newValue < lastOutput
→ frozen

else
→ đưa vào heap

Frozen records dùng chung vùng bộ nhớ với heap.

Giới hạn bộ nhớ:

heap + frozen ≤ heapCapacity

Output Buffer

Record lấy từ heap sẽ được ghi vào output buffer.

Luồng dữ liệu:

Heap → Output Buffer → Disk

Khi output buffer đầy:

ghi block ra disk
stats.addWrite()

Luồng hoạt động của Replacement Selection

Thuật toán hoạt động như sau:

Đọc dữ liệu ban đầu từ input và đưa vào heap cho đến khi heap đầy.

Lặp lại các bước sau:

Lấy record nhỏ nhất từ heap

Ghi vào output buffer

Đọc record tiếp theo từ input

Nếu:

record ≥ lastOutput
→ đưa vào heap

Nếu:

record < lastOutput
→ đưa vào frozen

Khi heap rỗng:

run hiện tại kết thúc

heap ← frozen
frozen ← rỗng

Bắt đầu tạo run mới.

Độ dài run trung bình

Replacement Selection giúp tạo run dài hơn kích thước RAM.

Trung bình:

run length ≈ 2 × heap size

tức là run có thể dài khoảng gấp đôi bộ nhớ sử dụng cho heap.

2. Multi-way Merge

Sau khi tạo các run ban đầu, thuật toán thực hiện multi-way merge để trộn các run lại thành một file đã sắp xếp.

Cấu trúc RAM trong giai đoạn merge:

B-1 input buffer
1 output buffer

Tổng cộng:

B buffer

Fan-in

Fan-in của thuật toán là:

fan-in = B - 1

Nghĩa là có thể merge B-1 run cùng lúc.

Ví dụ:

B = 5

4 input run
1 output buffer

→ merge 4-way

Quy trình merge

Mỗi run sẽ có một input buffer.

Bước đầu tiên:

đọc block đầu tiên của mỗi run vào buffer.

Sau đó lặp lại:

Chọn record nhỏ nhất trong các input buffer

Ghi record đó vào output buffer

Di chuyển con trỏ trong buffer đó

Nếu buffer đọc hết:

đọc block tiếp theo từ run đó.

Khi output buffer đầy:

ghi block ra disk.

Mô hình I/O

Mọi thao tác đọc và ghi disk đều thực hiện theo block.

Đọc:

stats.addRead()

Ghi:

stats.addWrite()

Nhờ đó chương trình có thể thống kê chi phí I/O:

Total IO = Disk Reads + Disk Writes

Tóm tắt

External Merge Sort gồm hai giai đoạn chính.

Giai đoạn 1 – Replacement Selection

Tạo các run đã được sắp xếp.

RAM sử dụng:

1 input buffer
1 output buffer
B-2 buffer cho heap

Giai đoạn 2 – Multi-way Merge

Merge các run lại thành file đã sắp xếp.

RAM sử dụng:

B-1 input buffer
1 output buffer

Thiết kế này giúp:

tận dụng tối đa bộ nhớ

giảm số lần truy cập disk

tối ưu chi phí I/O cho dữ liệu lớn.