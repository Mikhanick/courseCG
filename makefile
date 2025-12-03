# === Настройки ===
PDF_ROOT_DIR := report
OUTPUT_PROG_DIR ?= ./ready
SHELL_FORMAT_DIR ?= .
PDF_COMPRESS_DIR ?= $(OUTPUT_PROG_DIR)
READY_DIR := ./ready

GIT = git
SHELLCHECK = shellcheck
SHFMT = shfmt
GS = gs

# Цвета
GREEN = \033[32m
RED = \033[31m
YELLOW = \033[33m
BLUE = \033[34m
RESET = \033[0m

.PHONY: all clean ensure-output-dir \
	format-shell check-shell format-rust \
	compress-pdfs \
	cargo-test \
	check-git-changes check-dev-branch \
	auto-commit-if-cleanable push coverage \
	open-pdf lint-tex

# === Основная сборка ===
all: app-cli-debug app-cli-release app-tui-debug app-tui-release 

# ####### Создание Исполняемых ########
ensure-output-dir:
	mkdir -p $(OUTPUT_PROG_DIR)

%app-cli-debug: target/debug/cli ensure-output-dir
	cp $< $@

%app-cli-release: target/release/cli ensure-output-dir
	cp $< $@

%app-tui-debug: target/debug/tui ensure-output-dir
	cp $< $@

%app-tui-release: target/release/tui ensure-output-dir
	cp $< $@

target/debug/cli target/debug/tui:
	cargo build

target/release/cli target/release/tui:
	cargo build --release

# === Работа с отчётом ===
$(READY_DIR):
	mkdir -p $@

$(READY_DIR)/report.pdf: $(PDF_ROOT_DIR)/build/report.pdf | $(READY_DIR)
	cp $< $@

$(PDF_ROOT_DIR)/build/report.pdf:
	$(MAKE) -C $(PDF_ROOT_DIR) pdf

open-pdf:
	$(MAKE) -C $(PDF_ROOT_DIR) open-pdf

# === Очистка ===
clean:
	cargo clean
	rm -rf $(OUTPUT_PROG_DIR)
	rm -rf $(READY_DIR)
	$(MAKE) -C $(PDF_ROOT_DIR) clean

# === Форматирование и проверка shell-файлов ===
format-shell:
	@echo -e "$(BLUE)Форматирование shell-файлов в $(SHELL_FORMAT_DIR)...$(RESET)"
	@shell_files=$$(find $(SHELL_FORMAT_DIR) -type f \( -name "*.sh" -o -name "*.bash" \)); \
	if [ -z "$$shell_files" ]; then \
		echo -e "$(YELLOW)Shell-файлы не найдены.$(RESET)"; \
	else \
		for f in $$shell_files; do \
			$(SHFMT) -s -w "$$f"; \
		done; \
		echo -e "$(GREEN)Shell-файлы отформатированы.$(RESET)"; \
	fi

check-shell: format-shell
	@echo -e "$(BLUE)Проверка shell-файлов через shellcheck...$(RESET)"
	@shell_files=$$(find $(SHELL_FORMAT_DIR) -type f \( -name "*.sh" -o -name "*.bash" \)); \
	if [ -n "$$shell_files" ]; then \
		for f in $$shell_files; do \
			echo "Проверка $$f"; \
			$(SHELLCHECK) --color=always --norc --severity=error "$$f" || { echo -e "$(RED)Критические ошибки в $$f$(RESET)"; exit 1; }; \
			$(SHELLCHECK) --color=always --norc --severity=warning "$$f" || { echo -e "$(YELLOW)Предупреждения в $$f$(RESET)"; }; \
		done; \
	fi
	@echo -e "$(GREEN)Shell-файлы прошли проверку.$(RESET)"

# === Форматирование и проверка Rust-файлов ===
format-rust:
	@echo -e "$(BLUE)Форматирование Rust-файлов...$(RESET)"
	@if command -v rustfmt >/dev/null 2>&1; then \
		find . -name "*.rs" | xargs rustfmt; \
		echo -e "$(GREEN)Rust-файлы отформатированы.$(RESET)"; \
	else \
		echo -e "$(RED)rustfmt не найден.$(RESET)"; \
		exit 1; \
	fi

# === Сжатие PDF ===
compress-pdfs: $(READY_DIR)/report.pdf
	@echo -e "$(BLUE)Сжатие финального PDF-отчёта...$(RESET)"
	@tmp_file="$$(mktemp)"; \
	$(GS) -sDEVICE=pdfwrite -dCompatibilityLevel=1.4 -dNOPAUSE -dQUIET -dBATCH \
		-sOutputFile="$$tmp_file" $(READY_DIR)/report.pdf; \
	orig_size=$$(stat -c%s "$(READY_DIR)/report.pdf"); \
	new_size=$$(stat -c%s "$$tmp_file"); \
	if [ $$new_size -lt $$orig_size ]; then \
		mv "$$tmp_file" $(READY_DIR)/report.pdf; \
		echo -e "$(GREEN)Сжат: $(READY_DIR)/report.pdf ($$orig_size → $$new_size байт)$(RESET)"; \
	else \
		rm -f "$$tmp_file"; \
		echo -e "$(BLUE)Пропущен: $(READY_DIR)/report.pdf (сжатие не уменьшает размер)$(RESET)"; \
	fi

# === Тесты ===
cargo-test:
	@echo -e "$(BLUE)Запуск модульных тестов через cargo test...$(RESET)"
	cargo test -- --nocapture
	@echo -e "$(GREEN)Тесты пройдены успешно.$(RESET)"

# === Git: проверки ===
check-dev-branch:
	@echo -e "$(BLUE)Проверка текущей ветки...$(RESET)"
	@if ! $(GIT) branch --show-current | grep -q "^dev$$"; then \
		current_branch=$$($(GIT) branch --show-current); \
		echo -e "$(RED)Ошибка: вы не в ветке 'dev'. Текущая ветка: $$current_branch$(RESET)"; \
		exit 1; \
	fi
	@echo -e "$(GREEN)Ветка 'dev' активна.$(RESET)"

check-git-changes:
	@echo -e "$(BLUE)Проверка незакоммиченных изменений...$(RESET)"
	@if ! $(GIT) diff-index --quiet HEAD --; then \
		echo -e "$(YELLOW)Обнаружены незакоммиченные изменения.$(RESET)"; \
		$(GIT) diff --name-only; \
	else \
		echo -e "$(GREEN)Изменений нет.$(RESET)"; \
	fi

# === Автокоммит "чистых" изменений ===
auto-commit-if-cleanable:
	@echo -e "$(BLUE)Анализ изменений для автокоммита...$(RESET)"
	@if $(GIT) diff-index --quiet HEAD --; then \
		echo -e "$(GREEN)Нет изменений — коммит не требуется.$(RESET)"; \
	else \
		changed_files=$$($(GIT) diff --name-only); \
		cleanable=true; \
		non_cleanable_files=""; \
		for f in $$changed_files; do \
			case "$$f" in \
				*.sh|*.bash|*.pdf|Makefile|*.mk) \
					;; \
				*) \
					cleanable=false; \
					non_cleanable_files="$$non_cleanable_files $$f"; \
					;; \
			esac; \
		done; \
		if [ "$$cleanable" = false ]; then \
			echo -e "$(RED)Обнаружены значимые изменения в файлах:$$non_cleanable_files$(RESET)"; \
			echo -e "$(RED)Отправка запрещена. Закоммитьте вручную.$(RESET)"; \
			exit 1; \
		fi; \
		echo -e "$(GREEN)Изменения связаны только с форматированием/сжатием — обновляем последний коммит.$(RESET)"; \
		$(GIT) add .; \
		if $(GIT) log -1 >/dev/null 2>&1; then \
			$(GIT) commit --amend --no-edit --quiet; \
			echo -e "$(GREEN)Изменения добавлены в последний коммит (amend).$(RESET)"; \
		else \
			$(GIT) commit -m "chore: initial auto-format and PDF compression"; \
			echo -e "$(GREEN)Создан первый коммит в ветке.$(RESET)"; \
		fi; \
	fi

# === Полная отправка ===
push: check-dev-branch \
	  check-git-changes \
      check-shell \
	  format-rust \
	  clean \
	  $(READY_DIR)/report.pdf \
      compress-pdfs \
      cargo-test \
      auto-commit-if-cleanable 
	@echo -e "$(BLUE)Отправка в удалённый репозиторий...$(RESET)"
	$(GIT) push
	@echo -e "$(GREEN)✅ Пошла торпеда!$(RESET)"