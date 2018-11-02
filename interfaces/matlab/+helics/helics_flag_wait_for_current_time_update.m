function v = helics_flag_wait_for_current_time_update()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856897);
  end
  v = vInitialized;
end
