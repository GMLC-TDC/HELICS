function v = helics_flag_only_update_on_change()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183060);
  end
  v = vInitialized;
end
