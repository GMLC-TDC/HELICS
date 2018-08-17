function v = iteration_halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783851);
  end
  v = vInitialized;
end
