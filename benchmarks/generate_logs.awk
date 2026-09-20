BEGIN {
    srand(42)

    total = TOTAL
records_per_minute = 5000
    for (i = 0; i < total; i++) {

        minute_index = int(i / records_per_minute)

        hour = 10 + int(minute_index / 60)
        minute = minute_index % 60
        second = i % 60

        # Create an intentional error spike around minute 45.
        if (minute_index == 45) {
            error_probability = 0.50
        } else {
            error_probability = 0.05
        }

        r = rand()

        if (r < 0.80) {
            level = "INFO"
            component = "system"
            message = "Request processed successfully"
        }
        else if (r < 0.95) {
            level = "WARN"
            component = "database"
            message = "Query response slow"
        }
        else {
            level = "ERROR"

            if (i % 3 == 0) {
                component = "database"
                message = "Connection timeout"
            }
            else if (i % 3 == 1) {
                component = "model"
                message = "Model loading failed"
            }
            else {
                component = "auth"
                message = "Authentication failed"
            }
        }

        # Force a high number of errors during the spike.
        if (minute_index == 45) {
            if (r < error_probability) {
                level = "ERROR"

                if (i % 3 == 0) {
                    component = "database"
                    message = "Connection timeout"
                }
                else if (i % 3 == 1) {
                    component = "model"
                    message = "Model loading failed"
                }
                else {
                    component = "auth"
                    message = "Authentication failed"
                }
            }
        }

        printf "2026-09-19 %02d:%02d:%02d %s %s %s\n",
               hour, minute, second,
               level, component, message
    }
}