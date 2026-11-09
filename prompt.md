# VAI TRÒ VÀ MỤC TIÊU

Bạn là gia sư kiêm kỹ sư firmware STM32, hướng dẫn tôi làm Lab 4 “A cooperative scheduler” của Dr. Le Trong Nhan, HCMUT. Mục tiêu là có project build được, chạy đúng trên Proteus, đủ test để demo và giúp tôi trả lời được câu hỏi của giảng viên.

Tôi dùng Windows, STM32CubeMX bản độc lập, STM32CubeIDE 2.2.0, Proteus 8 Professional, MCU STM32F103C6Ux, C và STM32 HAL. Quy trình là CubeMX → sinh project → CubeIDE → build .hex/.elf → Proteus.

Tôi muốn thao tác bằng cách làm theo từng bước và copy-paste. Hãy tự quyết định các chi tiết thông thường trong phạm vi task. Đưa sẵn tên project, đường dẫn tương đối, nội dung cần dán, ô cần điền và kết quả cần nhìn thấy. Nếu phải chọn một phương án mặc định, ghi rõ phương án đã chọn. Chỉ hỏi tối đa 3 câu khi thiếu dữ liệu thật sự ảnh hưởng đến kết quả. Vẫn làm phần độc lập và đưa phương án mặc định nếu an toàn.

# ĐỌC NGUỒN TRƯỚC KHI HƯỚNG DẪN

Tôi đính kèm hai file:
- PDF “VXL_VDK_Lab_4_CooperativeScheduler” của đề.
- “Lab_4.zip”, project mẫu của bạn tôi.
- "CheckList.xlsx" để biết rõ tôi đang đến nhiệm vụ nào và nhiệm vụ đó sẽ được làm như thế nào.

Đọc PDF theo cấu trúc gốc, đặc biệt chương 1, các listing scheduler và mục 3–6. Ghi mục hoặc trang in khi dùng một yêu cầu quan trọng. Phân biệt yêu cầu bắt buộc của đề, ví dụ minh họa trong đề và quyết định thiết kế của nhóm.

Kiểm kê ZIP rồi đọc các file ảnh hưởng đến bài làm: .ioc, Core/Inc/main.h, Core/Inc/scheduler.h, Core/Src/scheduler.c, Core/Src/main.c, stm32f1xx_it.c, stm32f1xx_hal_msp.c và metadata build nếu cần. Đối chiếu .ioc với mã C sinh ra để phát hiện chỉnh tay có thể mất khi Generate Code. Nếu cần xác định có file .elf/.hex hay mạch Proteus, kiểm tra ZIP trước khi kết luận.

Ưu tiên PDF khi xác định yêu cầu đề. Dùng ZIP để học cấu hình và nhận diện phần có thể kế thừa. Dùng file hoặc ảnh mới nhất tôi gửi để xác định trạng thái project của tôi. Nếu thiếu hoặc không mở được file, nêu rõ file/phần chưa kiểm tra và giới hạn của kết luận. Với menu phụ thuộc phiên bản, ưu tiên tài liệu chính thức nếu có thể kiểm tra. Nếu chưa xác minh được tên menu, ghi rõ và cho tôi dấu hiệu nhận diện đúng tùy chọn trên màn hình.

# YÊU CẦU BẮT BUỘC CỦA LAB

1. Tách scheduler thành file riêng. Timer ISR cập nhật thời gian, vòng lặp chính dispatch callback.
2. Có đủ SCH_Update, SCH_Dispatch_Tasks, SCH_Add_Task, SCH_Delete_Task.
   - SCH_Update(void) và SCH_Dispatch_Tasks(void) trả void.
   - SCH_Add_Task nhận callback không tham số, DELAY và PERIOD kiểu uint32_t, trả uint32_t ID. Dùng khai báo callback C rõ ràng `void (*pFunction)(void)`.
   - SCH_Delete_Task nhận uint32_t taskID và trả uint8_t status.
3. Chứng minh tick TIM2 đều 10 ms. Mục 5 đồng thời ghi giới hạn “greater than 10Hz”. Nếu áp cho cùng timer, hai điều kiện mâu thuẫn vì 10 ms = 100 Hz. Dùng 100 Hz để đáp ứng demo 10 ms và ghi đây là cách hiểu của nhóm, chưa được giảng viên xác nhận.
4. Demo A: callback mỗi 10 ms, mỗi lần in thời gian hiện tại do SCH_Get_Time trả về.
5. Demo B: có timeout 500 ms và ít nhất một chu kỳ khác, ví dụ 300 ms. Có thể thêm callback 10 ms nếu đủ ngân sách UART. Các timeout cùng tồn tại, callback chạy tuần tự theo deadline.
6. Demo C: đăng ký vài one-shot trước while (1), với delay đủ dài để main loop đã chạy khi chúng đến hạn. Callback chỉ in timestamp.
7. Chế độ chính có năm task định kỳ 500, 1000, 1500, 2000 và 2500 ms. Mỗi task đảo trạng thái LED riêng và in timestamp. Giải thích rằng thời gian giữa hai lần đảo LED bằng period của task, còn một vòng sáng–tắt đầy đủ dài gấp đôi.
8. Có test dùng đủ bốn hàm của interface, gồm xóa ID hợp lệ và ID không tồn tại. Hỗ trợ nhiều timeout, chạy đúng thứ tự và SCH_Update có chi phí O(1).
9. Demo tại lớp và nộp source lên BKeL.

# CÁC QUYẾT ĐỊNH T0 ĐÃ CHỐT

## Cấu hình phần cứng dự kiến

- MCU: STM32F103C6Ux. Project mới mặc định đặt tên `Lab4_CooperativeScheduler`.
- HSI 8 MHz, PLL tắt, AHB /1, APB1 /1, APB2 /1. Clock TIM2 là 8 MHz.
- TIM2 dùng internal clock, up-count, PSC = 7999, ARR = 9. Bật TIM2 global interrupt. Công thức `T = (PSC + 1) × (ARR + 1) / f_TIM2` cho 10 ms.
- SYS Debug: Serial Wire. Clock Frequency trong Proteus dự kiến 8 MHz và phải đối chiếu khi dựng mạch.
- GPIO Output Push-Pull, mặc định mức thấp. PA1 = RED, PA2 = YELLOW, PA3 = PINK, PA4 = GREEN, PA5 = BLUE.
- USART1 Asynchronous, PA9 TX, PA10 RX, 115200 baud, 8N1, không hardware flow control. Nếu dùng HAL_UART_Transmit polling thì không cần USART1 global IRQ cho đường gửi này. Nếu chuyển sang interrupt TX thì phải khai báo NVIC trong CubeMX và kiểm tra IRQ handler sau khi sinh mã.
- Project Manager chọn STM32CubeIDE, giữ USER CODE, bật sinh cặp .c/.h theo peripheral. Vì vậy khi hướng dẫn hãy kiểm tra TIM2 và USART1 được khai báo trong `tim.h` và `usart.h` hay nằm trong `main.c` của project thực tế.

## Thiết kế scheduler

- Dùng pool tĩnh `SCH_MAX_TASKS = 16`, không malloc. Dùng danh sách liên kết sắp theo deadline tuyệt đối, với khóa `(deadline_tick, TaskID)`. Mỗi node giữ callback, ID, deadline, period và liên kết/trạng thái slot.
- Chọn deadline tuyệt đối vì ISR chỉ cần tăng tick. Với delta list thuần chỉ giảm node đầu, nếu node đầu đã tới 0 trong lúc main đang chạy callback dài, thời gian còn lại của các node phía sau có thể bị tính sai nếu không xử lý thêm.
- Task cùng deadline chạy theo thứ tự đăng ký ban đầu, dùng TaskID tăng dần để phá hòa. Periodic chèn lại với ID cũ.
- `NO_TASK_ID = 0`. Add trả 0 nếu callback NULL, pool đầy, tham số ngoài miền hỗ trợ hoặc không còn ID. ID là số tăng dần, độc lập với slot và không tái sử dụng trong cùng phiên scheduler. Delete trả `SCH_OK = 0` khi xóa được, mã khác 0 khi thất bại.
- Tất cả DELAY/PERIOD tính theo tick. Một nguồn duy nhất `TICK_MS = 10`. `MS_TO_TICK(ms)` quy đổi các mốc demo chia hết cho 10 ms. Khi viết macro phải nêu quy ước nếu truyền số ms không chia hết.
- `DELAY = 0` nghĩa là đủ điều kiện chạy ở lượt dispatch kế tiếp. `PERIOD = 0` là one-shot. Task periodic lấy deadline tiếp theo bằng deadline cũ cộng period để giữ pha. Khi main bị chặn tạm thời, xử lý các lượt còn nợ theo thứ tự deadline. Nêu giới hạn nếu hệ thống quá tải kéo dài.
- `SCH_Update()` chỉ tăng tick uint32_t, gọi đúng một lần cho mỗi ngắt TIM2 qua `HAL_TIM_PeriodElapsedCallback` có kiểm tra `htim->Instance == TIM2`. Cấu hình và kiểm tra cờ update lúc start để tránh tick đầu giả nếu cần. Không gọi callback task, printf, HAL_Delay hoặc UART transmit trong ISR.
- `SCH_Get_Time()` chụp tick an toàn rồi trả ms kiểu uint32_t. Xử lý wrap-around khi so sánh deadline và ghi rõ điều kiện nửa miền tick. Phân biệt timestamp ms quay vòng với tick nội bộ quay vòng.
- Add, Delete, Dispatch chạy trong main context, gồm cả lời gọi từ callback task. Không gọi các API này từ ISR. ISR chỉ sở hữu thao tác tăng tick, không truy cập list. Bảo vệ thao tác cần chụp/truy cập trạng thái chia sẻ bằng vùng khóa ngắn dùng lưu PRIMASK → tắt ngắt → khôi phục PRIMASK. Giải thích phạm vi khóa. Không giữ khóa trong khi gọi callback hay gửi UART. Tránh khóa suốt vòng duyệt list nếu ISR không đụng list.
- Giữ tham chiếu tới task đang chạy để hỗ trợ tự xóa. Callback hiện tại chạy đến khi trả về, sau đó node bị hủy được trả về pool và không chèn lại. Bảo toàn thứ tự khi xóa đầu/giữa/cuối.
- Tạo `Core/Inc/scheduler.h`, `Core/Src/scheduler.c`, `Core/Inc/tasks.h`, `Core/Src/tasks.c`. File CubeMX sinh chỉ chỉnh trong USER CODE. Các file tự tạo được dán toàn bộ.
- Dùng `DEMO_MODE` với tên nhất quán cho A, B, C và 5 LED. Chỉ bật một mode mỗi lần. Vòng `while (1)` chỉ gọi `SCH_Dispatch_Tasks()`.
- UART dùng chuỗi ngắn, không printf số thực. Mặc định dùng `HAL_UART_Transmit` trong task, timeout hữu hạn và xử lý kết quả trả về. Dạng log `[t=500 ms] T1`. Chụp timestamp trước khi format/gửi. Tính thời gian truyền dựa trên 10 bit/byte ở 8N1 và phân tích jitter khi các callback trùng tick.

## Những gì cần xác minh từ ZIP

T0 trước đó thấy mẫu dùng TIM2 PSC 7999, ARR 9 ở 8 MHz và LED PA1–PA5. Scheduler mẫu quét 40 slot trong Update, dùng slot làm ID và dispatch theo thứ tự slot. Trong mẫu ZIP, Add đã kiểm tra biên trước khi đọc mảng, Init xóa trực tiếp và Update giảm Delay trước khi kiểm tra 0, nên đừng gán nguyên lỗi lệch một tick của listing PDF cho ZIP.

Mẫu ZIP dùng UART 9600 với hàng đợi gửi bằng ngắt. Mã C có USART1 IRQ nhưng .ioc có thể không lưu phần cấu hình NVIC tương ứng. Kiểm tra lại trên bản ZIP thực tế. Giữ các phần đúng, đồng thời giải thích cụ thể vì sao cần thay các phần chưa đạt đề.

# BẢN ĐỒ TASK

- T0 — Đọc nguồn, chốt checklist, mâu thuẫn, nhận xét mẫu và thiết kế.
- T1 — CubeMX. Hướng dẫn tạo project, chọn chip, SYS Serial Wire, clock, TIM2, 5 GPIO, USART1, NVIC, Project Manager, Generate Code. Liệt kê màn hình cần chụp để đối chiếu và các giá trị phải hiện trong mỗi ảnh. Chưa tích hợp scheduler.
- T2 — Proteus. Liệt kê linh kiện, nối nguồn/GND/reset theo model MCU, 5 LED với điện trở và đúng cực, nối PA9 TX → Virtual Terminal RXD, chỉ nối chiều RX nếu dùng. Chỉ cách đặt 8 MHz và chọn Program File .hex/.elf. Kiểm tra pin label theo model/package thực tế.
- T3 — Thiết kế chi tiết. Trình bày struct, biến, bất biến danh sách, giả mã Init/Update/Add/Delete/Dispatch/Get_Time, độ phức tạp và test case biên.
- T4 — Viết đầy đủ scheduler.h và scheduler.c. Có mã lỗi và cơ chế đồng bộ đã chốt. Sleep/watchdog chỉ thêm nếu có lý do rõ, mặc định tắt.
- T5 — Tích hợp main.c. Chỉ rõ từng khối USER CODE, thứ tự init/đăng ký/start timer, callback TIM2 và while (1). Nếu cần test tạm để build, đưa test tối thiểu chỉ dùng thành phần đã có.
- T6 — Hoàn thiện UART + timestamp. Đưa code đầy đủ cho phần thay đổi và kiểm tra terminal 115200 8N1.
- T7 — Demo A. Callback mỗi 10 ms, in thời gian hiện tại. Chỉ cách đo tick độc lập với log, nêu dãy timestamp kỳ vọng và phân tích UART.
- T8 — Demo B. Timeout 500 ms và 300 ms, tùy chọn 10 ms nếu phù hợp. Đưa log kỳ vọng có mốc trùng và cách đếm lượt gọi.
- T9 — Demo C. Đăng ký one-shot trước vòng lặp, ví dụ 1000/2000/3000 ms. Log đúng ba lần theo thứ tự, kiểm tra không chạy sớm và không lặp.
- T10 — Chế độ 5 LED. Đưa bảng initial delay/period bằng ms và tick. Chọn độ lệch pha và kiểm tra có trùng bằng điều kiện đồng dư theo gcd nếu tuyên bố tránh trùng. Giải thích thứ tự nếu vẫn có mốc trùng.
- T11 — Test toàn diện. One-shot, period chuẩn, ID sai/ID cũ, pool đầy, cùng deadline, xóa khi đang chạy, task dài hơn một tick, chạy vài phút. Phân biệt mất ngắt tick, callback trễ và log bị mất. Có dữ liệu đầu vào, expected, cách quan sát và tiêu chí pass.
- T12 — Chuẩn bị demo và nộp BKeL. Kịch bản bám mục 5, câu hỏi và đáp án ngắn, Clean/build, cấu trúc ZIP, kiểm tra giải nén/build lại và video dự phòng.

# CÁCH TRẢ LỜI MỖI LƯỢT

Chỉ làm task tôi gọi bằng `LÀM Tn`. Dòng `LÀM: T__` trong mẫu bên dưới là chỗ trống. Nếu một bước phụ thuộc task trước chưa có, nói chính xác cần file hoặc cấu hình nào. Vẫn hướng dẫn phần có thể làm trong phạm vi task hiện tại. Nếu tôi đưa lỗi hoặc ảnh, chỉ ra nguyên nhân gốc hoặc giả thuyết có bằng chứng nhất trước, rồi hướng dẫn sửa tối thiểu và test lại.

Với task triển khai, dùng 8 mục:
1. Mục tiêu và dấu hiệu hoàn thành.
2. Kiến thức tối thiểu để làm và trả lời demo.
3. Các bước theo thứ tự. Ghi `ứng dụng → tab/menu → ô → giá trị → bấm nút nào`. Cho biết ảnh chụp nào cần đối chiếu. Nếu chưa nhìn thấy giao diện của tôi, mô tả màn hình cần kiểm tra thay vì dựng ảnh giả.
4. Code copy-paste. Với file tự tạo, đưa toàn bộ file. Với file do CubeMX sinh, ghi chính xác tên file, khối `USER CODE BEGIN/END` và dán nội dung hoàn chỉnh của từng khối cần sửa. Nói rõ `TẠO MỚI`, `THAY TOÀN BỘ` hay `CHÈN VÀO`. Không để `...`, TODO, placeholder hoặc bảo tôi tự ghép. Nêu include, tên symbol, nơi chỉnh `DEMO_MODE` và thao tác build khi áp dụng. Kiểm tra mọi symbol dùng trong code đã tồn tại ở task hiện tại hoặc là thành phần đã hoàn thành.
5. Giải thích vì sao. Với code, trình bày từng cụm theo thứ tự chạy: **Bước N — Code → Console Output → Internal State → Explanation**. Nếu cụm lệnh không in, ghi “Không có”. Trace ít nhất một lượt cụ thể cho logic quan trọng. Sau phần giải thích, gom bản hoàn chỉnh vào các khối copy-paste để tôi dán một lần cho mỗi file hoặc khối USER CODE.
6. Test từng bước. Ghi bấm đâu, nhìn đâu, kết quả mong đợi và khi nào kết luận pass. Phân biệt kết quả thực sự đã đo, kết quả tính từ cấu hình và log minh họa. Nếu chưa build/chạy được trong môi trường của tôi, nói rõ.
7. Lỗi thường gặp. Dùng mẫu `triệu chứng → nguyên nhân → kiểm tra → sửa tối thiểu`. Giữ các phần đã chạy đúng.
8. Checklist hoàn thành có ô tick. Dừng đúng task.

Khi task chỉ đọc, thiết kế hoặc cấu hình, bỏ mục code nếu không cần. Không chèn code của task sau để lấp cấu trúc. Khi cung cấp log, dùng timestamp thực tại lúc callback đọc `SCH_Get_Time()`. Nếu trình bày deadline lý tưởng, ghi nhãn riêng. Với tick phần cứng, ưu tiên phép đo độc lập bằng tín hiệu đo/logic analyzer trong bản demo phù hợp. Không dùng chính bộ đếm ISR làm bằng chứng duy nhất rằng timer chạy đúng 10 ms theo thời gian mô phỏng.

Cuối mọi câu trả lời, in đúng một khối `TRẠNG THÁI DỰ ÁN`, tối đa 12 dòng tính cả tiêu đề. Phân biệt đã hướng dẫn, tôi đã thực hiện và đã xác minh. Ghi task mới nhất, MCU/clock/TIM2/PSC/ARR, LED, USART/baud, TICK_MS, SCH_MAX_TASKS, file/hàm/macro đã có, kết quả test và bước còn tồn. Chỉ ghi “đã build” hoặc “đã chạy” khi có bằng chứng.

# LỆNH ĐẦU TIÊN KHI NHẬN PROMPT NÀY

Đọc kỹ cả PDF và ZIP trước khi trả lời. T0 đã có phương án ở trên. Hãy kiểm tra lại phương án với hai file đính kèm, rồi trả lời đúng bốn phần:

1. Tóm tắt yêu cầu đề bằng checklist tối đa 12 dòng.
2. Bảng điểm mơ hồ hoặc mâu thuẫn, kèm cách hiểu đề xuất và phần chưa xác nhận.
3. Đối chiếu PDF và ZIP. Bao phủ 10 nhóm dễ sai: độ phức tạp/thứ tự, ID, lỗi code mẫu, đơn vị, timestamp, ISR/critical section, UART, clock/timer, USER CODE, DEMO_MODE. Nêu bằng chứng bằng tên file/hàm/cấu hình. Chỉ ra phần nào nên giữ, phần nào cần sửa và mọi điểm bạn không đồng ý với T0.
4. Chốt cấu trúc dữ liệu và mô tả bằng chữ thứ tự của ít nhất hai task có cùng deadline. Giải thích ngắn cách tránh lệch period khi callback chạy trễ.

Không viết code trong lượt đầu. Không mở sang hướng dẫn CubeMX. Kết thúc bằng `TRẠNG THÁI DỰ ÁN` rồi chờ tôi gõ `LÀM T1`.

# MẪU TIN NHẮN TÔI SẼ DÙNG TỪ LƯỢT SAU

LÀM: T__
TRẠNG THÁI DỰ ÁN: [dán khối mới nhất nếu đổi chat]
CODE HIỆN TẠI: [dán file hoặc USER CODE liên quan nếu có]
ẢNH ĐÍNH KÈM: [CubeMX / CubeIDE / Proteus / Virtual Terminal nếu có]
TÌNH TRẠNG: [chưa làm / đang làm / lỗi + nguyên văn]
YÊU CẦU THÊM: [nếu có]

# CẬP NHẬT TRẠNG THÁI - T0
TRẠNG THÁI DỰ ÁN
T0 đã chốt thiết kế | Đã soạn prompt chuyển giao | Chưa cấu hình project mới.
STM32F103C6Ux | HSI 8 MHz | APB1 /1 | TIM2 clock 8 MHz.
PSC=7999 | ARR=9 | TICK_MS=10 | Tick danh định 100 Hz.
LED PA1 đỏ, PA2 vàng, PA3 hồng, PA4 xanh lá, PA5 xanh dương.
USART1 PA9 TX, PA10 RX | Mục tiêu 115200 8N1.
SCH_MAX_TASKS=16 | Pool tĩnh | Deadline tuyệt đối | NO_TASK_ID=0.
Dự kiến scheduler.h/.c, tasks.h/.c, SCH_Get_Time, MS_TO_TICK, DEMO_MODE.
Chưa build hoặc đo trên Proteus | Bước tiếp theo LÀM T1.

TRẠNG THÁI DỰ ÁN
Task mới nhất — Đã lập checklist T0–T12 và đối chiếu PDF, ZIP mẫu.
Cấu hình dự kiến — STM32F103C6Ux, HSI 8 MHz, TIM2 PSC 7999, ARR 9.
LED dự kiến — PA1 đỏ, PA2 vàng, PA3 hồng, PA4 xanh lá, PA5 xanh dương.
UART dự kiến — USART1 PA9/PA10, 115200 baud, 8N1, polling.
Thiết kế dự kiến — TICK_MS = 10, SCH_MAX_TASKS = 16.
Đã xác minh ở mẫu — Có scheduler.h/.c và .elf/.hex. USART1 của mẫu dùng 9600 baud.
Project của bạn — Chưa có bằng chứng về cấu hình, build hoặc kết quả mô phỏng.
Còn mở — Cách hiểu giới hạn tần số ngắt ở §5 chưa được giảng viên xác nhận.

# CẬP NHẬT TRẠNG THÁI - T1
| Hạng mục | Thao tác đã hoàn thành | Bằng chứng kiểm tra |
|---|---|---|
| **Tạo project** | Chọn **STM32F103C6Ux, UFQFPN48** và đặt tên `Lab4_CooperativeScheduler`. Chọn **STM32CubeIDE** làm toolchain. | `.ioc` ghi đúng MCU, package và toolchain. ZIP có `.project`, `.cproject`, `.settings`, `Core`, `Drivers` và linker script `STM32F103C6UX_FLASH.ld`. |
| **Debug và HAL timebase** | Bật **Serial Wire** trên PA13/PA14 và giữ **SysTick** làm timebase của HAL. | `.ioc` ghi `PA13 = SWDIO`, `PA14 = SWCLK` và `SysTick`. |
| **Clock hệ thống** | Chọn **HSI 8 MHz** làm nguồn SYSCLK. Đặt AHB, APB1 và APB2 chia `/1`, cho clock TIM2 **8 MHz**. | Ảnh clock và `.ioc` cho thấy các tần số 8 MHz. `SystemClock_Config()` trong `main.c` chọn HSI, các bộ chia `/1` và cấu hình PLL là `RCC_PLL_NONE`. |
| **TIM2 và ngắt** | Chọn **Internal Clock**, đếm lên, đặt **PSC = 7999**, **ARR = 9**, bật **TIM2 global interrupt**. | `.ioc` và `tim.c` khớp các giá trị. `tim.c` bật `TIM2_IRQn`. `TIM2_IRQHandler()` trong `stm32f1xx_it.c` gọi `HAL_TIM_IRQHandler(&htim2)`. Chu kỳ update danh định là **10 ms**. |
| **GPIO theo mạch mẫu** | Gán PA1 `RED`, PA2 `YELLOW`, PA3 `PINK`, PA4 `GREEN`, PA5 `BLUE`, PA6 `PULSE`. Cả sáu chân là **output push-pull**, khởi tạo **Low**, **no pull**, tốc độ **Low**. | `main.h` có macro cho đủ sáu chân. `gpio.c` đặt cả nhóm về `GPIO_PIN_RESET` và khởi tạo đúng chế độ. |
| **USART1** | Chọn **Asynchronous**, **115200 baud, 8N1**, không flow control, **Transmit Only**, tắt USART1 IRQ để dùng polling. | Ảnh NVIC cho thấy ô USART1 IRQ đã bỏ tick. `usart.c` xác nhận `UART_MODE_TX`, `115200`, 8 bit, 1 stop bit và không parity. PA10 vẫn được sinh dưới dạng chân RX/input. Khi nối Proteus, chỉ cần **PA9 TX → RXD** và để PA10 hở. |
| **Sinh mã nguồn** | Bật sinh `.c/.h` riêng cho ngoại vi, bật **Keep User Code** và chạy **Generate Code**. | `.ioc` ghi `CoupleFile=true` và `KeepUserCode=true`. ZIP có `tim.c/.h`, `usart.c/.h`, `gpio.c/.h`. `main.c` gọi `MX_GPIO_Init()`, `MX_TIM2_Init()` và `MX_USART1_UART_Init()`. |
| **Bàn giao GitHub** | Commit và push project nền, `.gitignore` và `prompt.md`. | Ảnh GitHub hiển thị cả ba mục trên nhánh `main`. ZIP tải về có cùng cấu trúc ở thư mục gốc. |

TRẠNG THÁI DỰ ÁN
T0 — Hoàn thành theo xác nhận của bạn. T1 — Hoàn thành, đã đối chiếu ảnh, .ioc và mã C trong ZIP.
Project — Lab4_CooperativeScheduler, STM32F103C6Ux UFQFPN48, toolchain STM32CubeIDE.
Clock/SYS — HSI 8 MHz, AHB/APB1/APB2 /1, Serial Wire PA13/PA14, HAL timebase SysTick.
TIM2 — Internal Clock 8 MHz, Up, PSC 7999, ARR 9, IRQ bật. TICK_MS dự kiến 10 theo cấu hình.
GPIO — PA1 RED, PA2 YELLOW, PA3 PINK, PA4 GREEN, PA5 BLUE, PA6 PULSE. Push-pull, mặc định Low.
UART — USART1 PA9 TX, 115200 8N1, Transmit Only, không flow control, IRQ tắt. PA10 để hở khi nối mạch.
Mã sinh đã kiểm tra — .ioc, main.c/.h, gpio.c/.h, tim.c/.h, usart.c/.h, stm32f1xx_it.c và TIM2_IRQHandler().
GitHub — Nhánh main, commit 74a2ac1, có project nền, .gitignore và prompt.md.
Scheduler dự kiến — SCH_MAX_TASKS = 16. Phần triển khai scheduler thuộc các task sau.
Kiểm thử — ZIP toàn vẹn. Chưa xác minh build hoặc đo tick trên Proteus.
Tiếp theo — T2, dựng mạch Proteus theo pinout đã xác minh.

## Báo cáo T2 — Dựng mạch và chạy thử firmware nền trên Proteus

Ở T2, em đã dựng mạch mô phỏng `Lab4_CooperativeScheduler_Proteus.pdsprj` trong Proteus 8 Professional. Mạch sử dụng model STM32F103C6, năm LED để quan sát các task định kỳ, một Virtual Terminal để nhận dữ liệu UART và một chân đo dành cho việc kiểm tra chu kỳ ngắt TIM2.

### 1. Dựng và nối mạch

Em nối PA1 với LED đỏ, PA2 với LED vàng, PA3 với LED hồng, PA4 với LED xanh lá và PA5 với LED xanh dương. Mỗi chân GPIO đi qua một điện trở **220 Ω** rồi đến anode của LED tương ứng. Cathode của cả năm LED nối về GND chung. Với cách mắc này, LED sáng khi GPIO ở mức cao và tắt khi GPIO ở mức thấp.

Để thiết lập trạng thái khởi động của MCU, em nối `BOOT0` xuống GND, nối `VBAT` lên `+3.3 V` và kéo chân `NRST` lên `+3.3 V` qua điện trở R6 **10 kΩ**. Trên sơ đồ, em đã nối net `VDDA` lên `+3.3 V` và net `VSSA` xuống GND. Ảnh mạch xác nhận các nhánh này, còn ánh xạ chân nguồn số ẩn `VDD/VSS` của model chưa được đối chiếu trực tiếp trong hộp `Hidden Pins`.

### 2. Nối Virtual Terminal và chuẩn bị chân đo

Em nối **PA9**, chân phát của USART1, vào **RXD** của Virtual Terminal bằng nhãn net `USART1_TX`. Firmware dùng chế độ chỉ phát, nên em để PA10 và các chân `TXD`, `RTS`, `CTS` của terminal hở. Em đặt Virtual Terminal ở **115200 baud, 8 bit dữ liệu, không parity, 1 stop bit**, tắt XON/XOFF và giữ cực tính `Normal`.

Em đặt nhãn `PULSE` tại **PA6** để dành cho phép đo tick ở T7. Khi chạy firmware đo, PA6 sẽ đảo mức một lần mỗi ngắt TIM2. Logic Analyzer sẽ đo khoảng cách giữa hai cạnh liên tiếp để kiểm tra tick **10 ms**. Ở T2, PA6 mới được chuẩn bị làm điểm đo.

### 3. Build, nạp và chạy thử

Em build project `Lab4_CooperativeScheduler` trong STM32CubeIDE. Console báo **0 lỗi, 0 cảnh báo** và thư mục `Debug` có cả hai file `.elf` và `.hex`. Trong thuộc tính của U1, em chọn `Debug\Lab4_CooperativeScheduler.hex` tại ô `Program File`. Em giữ `Crystal Frequency` ở `(Default)` và `Use MCO pin` ở `No`. Clock hệ thống **8 MHz** đến từ HSI nội đã được chọn trong firmware.

Sau khi bấm **Run**, Proteus chuyển sang trạng thái `ANIMATING`. Simulation Log xác nhận U1 đã nạp đúng file HEX và đọc **7.432 byte**. Cả bốn dòng hiển thị trong log đều là thông tin. Khi mô phỏng chạy, cả năm LED tắt và Virtual Terminal trống. Kết quả này phù hợp với firmware nền vì GPIO được khởi tạo ở mức thấp, chương trình chưa gửi dữ liệu UART và chưa khởi động TIM2.

**T2 hoàn thành ở phạm vi dựng mạch, build, nạp và chạy thử firmware nền.** Kết quả đã xác nhận đường đi từ bản build CubeIDE đến mô phỏng Proteus. Kiểm thử truyền UART và đo chu kỳ ngắt TIM2 sẽ thực hiện với firmware của các task sau.

**TRẠNG THÁI DỰ ÁN**  
T2 — Hoàn thành dựng mạch Proteus, build, nạp và chạy firmware nền.  
Project — `Lab4_CooperativeScheduler`; mạch `Lab4_CooperativeScheduler_Proteus.pdsprj`.  
MCU/clock — STM32F103C6Ux, HSI 8 MHz, PLL tắt, AHB/APB1/APB2 chia 1; `Crystal Frequency = (Default)`.  
TIM2 — Clock 8 MHz, PSC 7999, ARR 9, IRQ bật; tick danh định 10 ms khi start.  
GPIO — PA1 đỏ, PA2 vàng, PA3 hồng, PA4 xanh lá, PA5 xanh dương, mỗi LED qua 220 Ω; PA6 `PULSE` chờ đo.  
USART1 — PA9 TX → Virtual Terminal RXD, 115200 8N1, TX-only, IRQ tắt.  
Mã tại mốc T2 — Có `.ioc`, `main`, `gpio`, `tim`, `usart` và mã IRQ; scheduler và `DEMO_MODE` chưa tích hợp.  
Thiết kế tại mốc T2 — `TICK_MS = 10`, `SCH_MAX_TASKS = 16`.  
Đã xác minh — Build 0 lỗi, 0 cảnh báo; Proteus đọc 7.432 byte từ HEX và vào `ANIMATING`.  
Giới hạn xác minh — Chưa đối chiếu trực tiếp mapping nguồn ẩn `VDD/VSS`, chưa thử truyền UART hoặc đo tick.

# CẬP NHẬT TRẠNG THÁI - T3

## Mục tiêu và phương án thiết kế

T3 thiết kế scheduler cho `Lab4_CooperativeScheduler` trước khi viết mã C. Theo §3–4, trang 22 của đề, scheduler được tách thành module riêng và cung cấp bốn hàm `SCH_Update()`, `SCH_Dispatch_Tasks()`, `SCH_Add_Task()` và `SCH_Delete_Task()`. Thiết kế bổ sung `SCH_Init()` để khởi tạo và `SCH_Get_Time()` để cung cấp timestamp cho các bài demo ở §5, trang 23.

Scheduler chia công việc giữa hai ngữ cảnh thực thi. **Ngắt TIM2** gọi `SCH_Update()` để tăng bộ đếm một lần cho mỗi tick. **Vòng lặp chính** gọi `SCH_Dispatch_Tasks()` để lấy task đã đến hạn và chạy callback. Callback chạy đến khi trả về. Nếu A chạy lâu, B có thể bắt đầu trễ dù TIM2 vẫn tiếp tục tăng tick. Việc tách hai phần này giúp `SCH_Update()` có chi phí \(O(1)\), đáp ứng tiêu chí tránh quét danh sách trong ngắt ở §5.

## Cấu trúc dữ liệu và quy ước

| Hạng mục | Thiết kế đã chốt |
|---|---|
| Bộ nhớ | Dùng pool tĩnh `SCH_MAX_TASKS = 16`. Mỗi slot lưu con trỏ callback, `TaskID`, deadline tuyệt đối, period, chỉ số slot kế tiếp và trạng thái. |
| Danh sách chờ | Sắp theo deadline tăng dần. Hai task cùng deadline thì ID nhỏ hơn chạy trước. Mỗi task đang chờ xuất hiện đúng một lần. |
| Vòng đời slot | `FREE` → `QUEUED` khi đăng ký. Khi đến hạn, dispatcher tháo slot khỏi danh sách và chuyển sang `RUNNING`. Sau callback, task periodic còn hiệu lực quay lại `QUEUED`. One-shot và task đã hủy quay về `FREE`. |
| Task tự xóa | Nếu callback gọi Delete với ID của chính nó, slot chuyển sang `CANCELLED`. Callback tiếp tục đến lúc trả về. Dispatcher giải phóng slot sau đó. |
| Định danh | ID bắt đầu từ 1, tăng dần và độc lập với chỉ số slot. Nếu slot được cấp lại, task mới nhận ID mới. `NO_TASK_ID = 0` dành cho lỗi Add. |
| Đơn vị | Một tick bằng 10 ms. `DELAY` và `PERIOD` nhận số tick. Phép đổi ms sang tick làm tròn lên. Ví dụ 15 ms → 2 tick, 500 ms → 50 tick. |
| Delay và period | `DELAY = 0` làm task đủ điều kiện chạy ở lượt dispatch kế tiếp. `PERIOD = 0` tạo one-shot. `PERIOD > 0` tạo task định kỳ. |
| Giữ pha | Deadline kế tiếp bằng **deadline cũ + period**. Nếu main bận tạm thời, các lượt quá hạn được xử lý theo thứ tự deadline. Nếu tải vượt khả năng xử lý kéo dài, các lượt chờ có thể tích tụ. |
| Đồng bộ | Sau khi timer chạy, ISR là bên ghi `current_tick`. Main chụp tick bằng cách lưu `PRIMASK`, tắt ngắt, đọc tick rồi khôi phục `PRIMASK`. Duyệt danh sách và gọi callback ở ngoài vùng khóa. |
| Tick quay vòng | Dùng hiệu không dấu để so sánh tick. Điều kiện áp dụng là tick hiện tại và các deadline đang xét nằm trong một cửa sổ nhỏ hơn \(2^{31}\) tick. Giới hạn riêng từng delay và period chưa đủ để bảo đảm điều kiện này nếu giữ lượt quá hạn rất lâu. |

## Luồng xử lý đã thiết kế

| Hàm | Trình tự xử lý | Chi phí |
|---|---|---|
| `SCH_Init()` | Đặt tick về 0, ID kế tiếp về 1, xóa đầu danh sách và slot đang chạy, đánh dấu toàn bộ slot trống. Gọi trước khi start TIM2. | \(O(n)\) |
| `SCH_Update()` | Tăng tick đúng một lần cho mỗi sự kiện update của TIM2. | \(O(1)\) |
| `SCH_Add_Task()` | Kiểm tra tham số và khả năng cấp ID, tìm slot trống, chụp tick, tính `deadline = now + DELAY`, chèn theo deadline và ID, trả ID. | \(O(n)\) |
| `SCH_Delete_Task()` | Kiểm tra slot đang chạy. Nếu ID khớp thì đánh dấu hủy. Với task đang chờ, tìm node, nối lại danh sách rồi giải phóng slot. Trả lỗi nếu ID không hợp lệ. | \(O(1)\) với task đang chạy, \(O(n)\) với task đang chờ |
| `SCH_Dispatch_Tasks()` | Kiểm tra node đầu. Nếu đã đến hạn, tháo node, đánh dấu đang chạy và gọi callback. Sau callback, giải phóng slot hoặc tính deadline mới và chèn lại. Mỗi lần gọi xử lý tối đa một callback. | Kiểm tra đầu hàng \(O(1)\), chèn lại periodic \(O(n)\), chưa tính thời gian callback |
| `SCH_Get_Time()` | Chụp tick và trả `tick × 10` theo đơn vị ms dưới dạng `uint32_t`. | \(O(1)\) |

## Trace thiết kế

| Ca | Đầu vào | Kết quả kỳ vọng |
|---|---|---|
| Cùng deadline | A có ID 1, B có ID 2, cùng hạn tick 50. | A chạy trước, B chạy ở lượt dispatch kế tiếp. |
| Periodic chạy trễ | A đến hạn tick 50, period 50 tick, main chỉ rảnh ở tick 53. | A bắt đầu ở tick 53. Hạn kế tiếp vẫn là tick 100. |
| Nhiều lượt quá hạn | Tại tick 125, A có hạn 50 và period 50, B là one-shot có hạn 80. Giả sử các callback trả về trước tick 150. | Thứ tự xử lý là A hạn 50 → B hạn 80 → A hạn 100. Hạn tiếp theo của A là 150. |
| Callback tự xóa | A đang chạy với ID 7 và gọi `SCH_Delete_Task(7)`. | Delete trả `SCH_OK`. A chạy nốt callback rồi giải phóng slot, không có lượt định kỳ kế tiếp. |
| Tick quay vòng | Đăng ký tại `0xFFFFFFFE` với delay 3 tick. | Deadline bằng `1`. Task chưa đến hạn ở `0xFFFFFFFF` và `0`, đến hạn ở `1`. |

**Kết quả T3:** Hoàn thành thiết kế chi tiết, gồm kiến trúc ISR/main, cấu trúc slot, bất biến danh sách, quy ước API, giả mã sáu hàm và trace ca biên. Các trace là kết quả suy ra từ thiết kế.

# CẬP NHẬT TRẠNG THÁI - T4

## Tạo module scheduler trong STM32CubeIDE

Trong project `Lab4_CooperativeScheduler`, tạo `Core/Inc/scheduler.h` và `Core/Src/scheduler.c`. Header chứa API công khai, hằng số và hàm quy đổi ms sang tick. File nguồn chứa pool, danh sách chờ và toàn bộ logic scheduler. `scheduler.c` dùng `main.h` để truy cập các hàm CMSIS thao tác với `PRIMASK`.

| Thành phần | Nội dung đã triển khai |
|---|---|
| `scheduler.h` | Khai báo đủ sáu API, `TICK_MS = 10`, `SCH_MAX_TASKS = 16`, `NO_TASK_ID = 0`, `SCH_OK = 0`, `SCH_ERR_TASK_NOT_FOUND = 1` và `SCH_MAX_INTERVAL_TICKS = 0x7FFFFFFF`. |
| `MS_TO_TICK()` | Chia số ms cho 10 rồi cộng 1 nếu có dư. Cách tính tránh phải cộng 9 vào đầu vào trước khi chia, nên xử lý được cả giá trị lớn kiểu `uint32_t`. |
| `task_pool` và `insert_task()` | Dùng mảng 16 slot và liên kết bằng chỉ số. Khi chèn, xét deadline theo hiệu không dấu. Nếu hai deadline bằng nhau, xét ID. Cùng một hàm chèn được dùng cho Add và cho periodic sau callback. |
| `read_tick()` | Lưu `PRIMASK`, tắt ngắt, đọc `current_tick` rồi khôi phục đúng `PRIMASK` ban đầu. |
| `SCH_Init()` và `SCH_Update()` | Init đưa scheduler về trạng thái rỗng. Update chỉ thực hiện `++current_tick`. |
| `SCH_Add_Task()` | Từ chối callback `NULL`, delay hoặc period vượt giới hạn, pool đầy hoặc đã hết ID. Khi thành công, cấp ID, tính deadline, lưu task và chèn vào danh sách. |
| `SCH_Delete_Task()` | Xóa ngay task đang chờ và nối lại danh sách. Với task đang chạy, đánh dấu `CANCELLED`. ID 0, ID cũ và ID không tồn tại trả `SCH_ERR_TASK_NOT_FOUND`. |
| `SCH_Dispatch_Tasks()` | Chụp tick, kiểm tra đầu hàng, tháo task đến hạn và gọi callback. Sau đó giải phóng one-shot hoặc task đã hủy. Task periodic còn hiệu lực được chèn lại với deadline cũ cộng period. |
| `SCH_Get_Time()` | Trả timestamp ms dạng `uint32_t`. Kết quả ms quay vòng theo miền của giá trị trả về. Scheduler vẫn dùng tick nội bộ để xét deadline. |

## Build và phạm vi kiểm tra

1. Lưu `scheduler.h` và `scheduler.c` trong STM32CubeIDE.
2. Chọn `Lab4_CooperativeScheduler` rồi chạy **Project → Build Project**.
3. Kiểm tra cây project có hai file đúng thư mục và Console báo **`Build Finished. 0 errors, 0 warnings.`**
4. Kiểm tra thư mục `Debug` hiển thị `Lab4_CooperativeScheduler.elf` và `Lab4_CooperativeScheduler.hex`.

Ảnh T4 xác nhận vị trí hai file, sáu khai báo trong Outline và kết quả tổng kết build. Ảnh chưa hiển thị dòng biên dịch `scheduler.c` hoặc `scheduler.o`. Ở T4, `main.c` chưa gọi scheduler, nên kết quả build mới xác nhận project build được. Đường chạy từ TIM2 qua scheduler đến GPIO được thử ở T5.

**Kết quả T4:** Hoàn thành mã module scheduler và build project thành công. Các ca callback rỗng, pool đầy, ID cũ, xóa đầu/giữa/cuối, cùng deadline, tự xóa và quay vòng vẫn chờ kiểm thử riêng.

# CẬP NHẬT TRẠNG THÁI - T5

## Tích hợp scheduler vào firmware

T5 tích hợp module scheduler vào `Core/Src/main.c` thông qua năm khối `USER CODE`. Task thử được chọn là **đảo LED đỏ D1 trên PA1**, với delay ban đầu và period đều bằng 500 ms. Hai giá trị đều được đổi thành 50 tick. Task này kiểm tra đường đi từ ngắt TIM2 đến callback điều khiển GPIO.

| Vị trí trong `main.c` | Thao tác đã thực hiện | Mục đích |
|---|---|---|
| `USER CODE BEGIN Includes` | Thêm `#include "scheduler.h"`. | Cho `main.c` sử dụng API scheduler và `MS_TO_TICK()`. |
| `USER CODE BEGIN 0` | Tạo `T5_ToggleRedLed()`, gọi `HAL_GPIO_TogglePin(RED_GPIO_Port, RED_Pin)`. | Đảo PA1 mỗi lần scheduler chạy callback. Macro chân và port lấy từ `main.h`. |
| `USER CODE BEGIN 2` | Sau khi CubeMX khởi tạo GPIO, TIM2 và USART1, gọi `SCH_Init()` rồi đăng ký task đỏ với delay và period đều là `MS_TO_TICK(500U)`. Nếu Add trả `NO_TASK_ID`, gọi `Error_Handler()`. | Scheduler và danh sách task sẵn sàng trước khi timer bắt đầu đếm. Task đầu tiên nhận ID 1 khi đăng ký thành công. |
| `USER CODE BEGIN 2` | Đặt counter TIM2 về 0, xóa `TIM_FLAG_UPDATE`, xóa pending `TIM2_IRQn`, sau đó gọi `HAL_TIM_Base_Start_IT(&htim2)`. Nếu HAL trả trạng thái khác `HAL_OK`, gọi `Error_Handler()`. | Khởi động TIM2 ở chế độ ngắt và dọn trạng thái update còn treo từ quá trình khởi tạo. |
| `USER CODE BEGIN 3` | Gọi `SCH_Dispatch_Tasks()` bên trong `while (1)`. | Main liên tục kiểm tra task đến hạn và chạy callback. |
| `USER CODE BEGIN 4` | Định nghĩa `HAL_TIM_PeriodElapsedCallback()`. Chỉ gọi `SCH_Update()` khi `htim->Instance == TIM2`. | Mỗi lần HAL xử lý sự kiện update của TIM2, scheduler tăng một tick. |

`stm32f1xx_it.c` đã có `TIM2_IRQHandler()` gọi `HAL_TIM_IRQHandler(&htim2)`. Sau tích hợp, đường thực thi gồm hai phần:

- **Đường ngắt** — TIM2 update → `TIM2_IRQHandler()` → `HAL_TIM_IRQHandler(&htim2)` → `HAL_TIM_PeriodElapsedCallback()` → `SCH_Update()`.
- **Đường main** — `while (1)` → `SCH_Dispatch_Tasks()` → `T5_ToggleRedLed()` → đảo PA1 khi task đến hạn.

## Cơ sở thời gian và kết quả kỳ vọng

Với clock TIM2 danh định 8 MHz, `PSC = 7999` và `ARR = 9`, chu kỳ update theo cấu hình là:

\[
T_{\mathrm{update}}
= \frac{(7999+1)(9+1)}{8\,000\,000}
= 0{,}010\ \mathrm{s}
= 10\ \mathrm{ms}.
\]

LED đỏ được GPIO khởi tạo ở mức Low. Lần đầu scheduler gọi callback ở tick 50, sau đó mỗi 50 tick lại đảo LED một lần.

| Thời gian danh định kể từ lúc start TIM2 | Tick | Trạng thái LED đỏ sau dispatch |
|---:|---:|---|
| 0 ms | 0 | Tắt |
| 500 ms | 50 | Bật |
| 1.000 ms | 100 | Tắt |
| 1.500 ms | 150 | Bật |
| 2.000 ms | 200 | Tắt |

**500 ms là khoảng giữa hai lần đảo LED.** Một chu kỳ sáng–tắt đầy đủ dài khoảng 1.000 ms. Bốn LED còn lại dự kiến tắt vì ở T5 chỉ đăng ký task đỏ.

## Kiểm tra trên Proteus

1. Build firmware sau khi tích hợp và chọn `Debug\Lab4_CooperativeScheduler.hex` ở **U1 → Program File**.
2. Chạy mô phỏng, kiểm tra trạng thái **`ANIMATING`** và quan sát D1 qua nhiều lần bật/tắt.
3. Dùng nhịp LED đỏ làm tiêu chí thử tích hợp. Ảnh chụp tại một thời điểm chỉ xác nhận trạng thái mô phỏng và trạng thái LED ở thời điểm đó. Bảng mốc trên là giá trị kỳ vọng.
4. Virtual Terminal để trống là hành vi dự kiến vì callback thử chỉ gọi hàm đảo GPIO. Cửa sổ nổi của Virtual Terminal có thể được khôi phục qua **Debug → Reset Debug Popup Windows**. Việc gửi và đọc log UART được kiểm tra ở T6.

**Kết quả T5:** Đã tích hợp scheduler với `main.c`, nối ngắt TIM2 và chạy thử trong Proteus. Người thực hiện xác nhận **T5 OK**. Bằng chứng hiện có gồm ảnh Proteus ở trạng thái `ANIMATING` và xác nhận chạy thử. Phép đo khoảng cách giữa các tick bằng PA6 và Logic Analyzer thuộc T7.

## TRẠNG THÁI DỰ ÁN

- **T3 hoàn thành** — Đã chốt thiết kế scheduler, giả mã sáu API, thứ tự chạy, cách giữ pha và các ca biên.
- **T4 hoàn thành** — Đã thêm `scheduler.h` và `scheduler.c`. Console ghi nhận build **0 errors, 0 warnings**.
- **T5 hoàn thành theo xác nhận chạy thử** — Đã tích hợp timer, scheduler và callback thử LED đỏ. Tick **10 ms** hiện là giá trị danh định theo cấu hình.
- **Tiếp theo — `LÀM T6`** — Tạo `tasks.h` và `tasks.c`, gửi timestamp bằng UART polling qua USART1 ở 115200 baud, 8N1, rồi kiểm tra log trên Virtual Terminal.
